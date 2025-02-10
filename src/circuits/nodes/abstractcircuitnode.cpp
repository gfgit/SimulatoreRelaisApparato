/**
 * src/circuits/nodes/abstractcircuitnode.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "abstractcircuitnode.h"

#include "../electriccircuit.h"
#include "circuitcable.h"

#include <QJsonObject>

AbstractCircuitNode::AbstractCircuitNode(ModeManager *mgr, bool isLoad, QObject *parent)
    : QObject{parent}
    , mModeMgr(mgr)
    , isElectricLoad(isLoad)
{

}

AbstractCircuitNode::~AbstractCircuitNode()
{
    Q_ASSERT(mClosedCircuits.isEmpty());
    Q_ASSERT(mOpenCircuits.isEmpty());

    // Detach all contacts
    for(int i = 0; i < mContacts.size(); i++)
    {
        detachCable(i);
    }
}

void AbstractCircuitNode::addCircuit(ElectricCircuit *circuit)
{
    CircuitList& circuitList = getCircuits(circuit->type());

    // A circuit may pass 2 times on same node
    // But we add it only once
    if(circuitList.contains(circuit))
        return;

    circuitList.append(circuit);

    bool updateNeeded = false;

    const auto items = circuit->getNode(this);
    for(int i = 0; i < items.size(); i++)
    {
        const NodeItem& item = items.at(i);

        if(item.fromContact != NodeItem::InvalidContact)
        {
            uint16_t &fromCount = mContacts[item.fromContact].entranceCount(circuit->type(),
                                                                            item.fromPole);
            fromCount++;
            if(fromCount == 1)
                updateNeeded = true;
        }

        if(item.toContact != NodeItem::InvalidContact)
        {
            uint16_t &toCount = mContacts[item.toContact].exitCount(circuit->type(),
                                                                    item.toPole);
            toCount++;
            if(toCount == 1)
                updateNeeded = true;
        }
    }

    if(circuitList.size() == 1 || updateNeeded)
        emit circuitsChanged();
}

void AbstractCircuitNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences& items)
{
    CircuitList& circuitList = getCircuits(circuit->type());
    Q_ASSERT(circuitList.contains(circuit));

    partialRemoveCircuit(circuit, items);

    circuitList.removeOne(circuit);
}

void AbstractCircuitNode::partialRemoveCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    Q_ASSERT(getCircuits(circuit->type()).contains(circuit));

    bool updateNeeded = false;

    for(int i = 0; i < items.size(); i++)
    {
        const NodeItem& item = items.at(i);

        if(item.fromContact != NodeItem::InvalidContact)
        {
            uint16_t &fromCount = mContacts[item.fromContact].entranceCount(circuit->type(),
                                                                            item.fromPole);
            Q_ASSERT(fromCount > 0);

            fromCount--;
            if(fromCount == 0)
                updateNeeded = true;
        }

        if(item.toContact != NodeItem::InvalidContact)
        {
            uint16_t &toCount = mContacts[item.toContact].exitCount(circuit->type(),
                                                                    item.toPole);
            Q_ASSERT(toCount > 0);

            toCount--;
            if(toCount == 0)
                updateNeeded = true;
        }
    }

    if(updateNeeded)
        emit circuitsChanged();
}

void AbstractCircuitNode::attachCable(const CableItem& item)
{
    // Either contact if free or we are adding poles to same cable
    Q_ASSERT(item.cable.cable->getNode(item.cable.side).node == this || !item.cable.cable->getNode(item.cable.side).node);
    Q_ASSERT(item.nodeContact < mContacts.size());
    Q_ASSERT(mContacts.at(item.nodeContact).cable == item.cable.cable || !mContacts.at(item.nodeContact).cable);
    Q_ASSERT(mContacts.at(item.nodeContact).getType(item.cable.pole) == ContactType::NotConnected);

    NodeContact& contact = mContacts[item.nodeContact];
    if(!contact.cable)
    {
        // Connect new cable
        contact.cable = item.cable.cable;
        contact.cableSide = item.cable.side;
        contact.setType(item.cable.pole, ContactType::Connected);

        CableEnd cableEnd;
        cableEnd.node = this;
        cableEnd.nodeContact = item.nodeContact;
        item.cable.cable->setNode(item.cable.side, cableEnd);
    }
    else
    {
        // Add a pole
        contact.setType(item.cable.pole, ContactType::Connected);
    }
}

void AbstractCircuitNode::detachCable(const CableItem &item)
{
    Q_ASSERT(item.cable.cable->getNode(item.cable.side).node == this);
    Q_ASSERT(item.nodeContact < mContacts.size());
    Q_ASSERT(mContacts.at(item.nodeContact).cable == item.cable.cable);
    Q_ASSERT(mContacts.at(item.nodeContact).getType(item.cable.pole) != ContactType::NotConnected);

    NodeContact& contact = mContacts[item.nodeContact];

    // Check other pole
    if(contact.getType(~item.cable.pole) != ContactType::Connected)
    {
        // Other pole is not connected, remove cable
        contact.cable = nullptr;
        item.cable.cable->setNode(item.cable.side, {});
        contact.setType(item.cable.pole, ContactType::NotConnected);
    }
    else
    {
        // Keep other pole
        contact.setType(item.cable.pole, ContactType::NotConnected);
    }
}

bool AbstractCircuitNode::loadFromJSON(const QJsonObject &obj)
{
    if(obj.value("type") != nodeType())
        return false;

    setObjectName(obj.value("name").toString());
    return true;
}

void AbstractCircuitNode::saveToJSON(QJsonObject &obj) const
{
    obj["type"] = nodeType();
    obj["name"] = objectName();
}

bool AbstractCircuitNode::isSourceNode(bool onlyCurrentState) const
{
    Q_UNUSED(onlyCurrentState)
    return false;
}

bool AbstractCircuitNode::sourceDoNotCloseCircuits() const
{
    return false;
}

bool AbstractCircuitNode::isSourceEnabled() const
{
    return false;
}

void AbstractCircuitNode::setSourceEnabled(bool newEnabled)
{
    Q_UNUSED(newEnabled)
}

bool AbstractCircuitNode::tryFlipNode(bool forward)
{
    return false;
}

void AbstractCircuitNode::detachCable(int contactIdx)
{
    NodeContact& contact = mContacts[contactIdx];
    if(contact.cable)
    {
        contact.cable->setNode(contact.cableSide, {});
        contact.type1 = contact.type2 = ContactType::NotConnected;
        contact.cable = nullptr;
    }
}

void AbstractCircuitNode::disableCircuits(const CircuitList &listCopy,
                                          AbstractCircuitNode *node)
{
    for(ElectricCircuit *circuit : listCopy)
    {
        Q_ASSERT(circuit->type() == CircuitType::Closed);
    }

    for(ElectricCircuit *circuit : listCopy)
    {
        circuit->disableOrTerminate(node);
    }
}

void AbstractCircuitNode::disableCircuits(const CircuitList &listCopy,
                                          AbstractCircuitNode *node,
                                          const int contact)
{
    for(ElectricCircuit *circuit : listCopy)
    {
        Q_ASSERT(circuit->type() == CircuitType::Closed);
    }

    for(ElectricCircuit *circuit : listCopy)
    {
        const auto items = circuit->getNode(this);
        for(const NodeItem& item : items)
        {
            if(item.fromContact == contact && item.toContact == NodeItem::InvalidContact)
                continue; // This circuit comes from outside

            if(item.fromContact == contact || item.toContact == contact)
            {
                circuit->disableOrTerminate(node);
                break;
            }
        }
    }
}

void AbstractCircuitNode::truncateCircuits(const CircuitList &listCopy,
                                           AbstractCircuitNode *node)
{
    CircuitList duplicateList;
    for(ElectricCircuit *circuit : listCopy)
    {
        circuit->terminateHere(node, duplicateList);
    }
}

void AbstractCircuitNode::truncateCircuits(const CircuitList &listCopy,
                                           AbstractCircuitNode *node,
                                           const int contact)
{
    CircuitList duplicateList;
    for(ElectricCircuit *circuit : listCopy)
    {
        const auto items = circuit->getNode(this);
        for(const NodeItem& item : items)
        {
            if(item.fromContact == contact && item.toContact == NodeItem::InvalidContact)
                continue; // This circuit comes from outside

            if(item.fromContact == contact || item.toContact == contact)
            {
                circuit->terminateHere(node, duplicateList);
                break;
            }
        }
    }
}

void AbstractCircuitNode::unregisterOpenCircuitExit(ElectricCircuit *circuit)
{
    Q_ASSERT(circuit->type() == CircuitType::Open);
    Q_ASSERT(mOpenCircuits.contains(circuit));
    Q_ASSERT(circuit->isLastNode(this));

    const auto item = circuit->getNode(this).last();

    if(item.toContact != NodeItem::InvalidContact)
    {
        // We enabled exit contact only for circuit passing through
        // Open circuits which end here (last node) do not enable toCount
        uint16_t &toCount = mContacts[item.toContact].exitCount(circuit->type(),
                                                                item.toPole);
        Q_ASSERT(toCount > 0);

        toCount--;
        if(toCount == 0)
            emit circuitsChanged();
    }
}
