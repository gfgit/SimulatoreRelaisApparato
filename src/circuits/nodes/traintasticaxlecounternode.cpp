/**
 * src/circuits/nodes/traintasticaxlecounternode.cpp
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

#include "traintasticaxlecounternode.h"

#include "../../objects/traintastic/traintasticaxlecounterobj.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../electriccircuit.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

TraintasticAxleCounterNode::TraintasticAxleCounterNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 3 sides
    mContacts.append(NodeContact("1", "2"));   // FreeTrackOut
    mContacts.append(NodeContact("11", "12")); // ResetIn
    mContacts.append(NodeContact("13", "14")); // OccupiedTrackOut
    mContacts.append(NodeContact("3", "4"));   // PowerIn
}

TraintasticAxleCounterNode::~TraintasticAxleCounterNode()
{
    setAxleCounter(nullptr);
}

AbstractCircuitNode::ConnectionsRes TraintasticAxleCounterNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(!mAxleCounter)
        return {};


    if(source.nodeContact == Contacts::ResetIn)
    {
        // Close the circuit on itself for reset input
        CableItemFlags dest;
        dest.cable.cable = mContacts.at(Contacts::ResetIn).cable;
        dest.cable.side = mContacts.at(Contacts::ResetIn).cableSide;
        dest.nodeContact = Contacts::ResetIn;
        dest.cable.pole = ~source.cable.pole; // Invert pole
        return {dest};
    }

    if(source.nodeContact == Contacts::PowerIn)
    {
        int destContact = NodeItem::InvalidContact;
        if(mAxleCounter->state() == TraintasticAxleCounterObj::State::Occupied ||
                mAxleCounter->state() == TraintasticAxleCounterObj::State::OccupiedAtStart)
        {
            destContact = Contacts::OccupiedTrackOut;
        }
        else if(mAxleCounter->state() == TraintasticAxleCounterObj::State::Free)
        {
            destContact = Contacts::FreeTrackOut;
        }

        if(destContact == NodeItem::InvalidContact)
            return {};

        CableItemFlags dest;
        dest.cable.cable = mContacts.at(destContact).cable;
        dest.cable.side = mContacts.at(destContact).cableSide;
        dest.nodeContact = destContact;
        dest.cable.pole = source.cable.pole;
        return {dest};
    }
    else
    {
        bool active = false;
        if(source.nodeContact == Contacts::FreeTrackOut &&
                mAxleCounter->state() == TraintasticAxleCounterObj::State::Free)
            active = true;
        else if(source.nodeContact == Contacts::OccupiedTrackOut &&
                (mAxleCounter->state() == TraintasticAxleCounterObj::State::Occupied ||
                 mAxleCounter->state() == TraintasticAxleCounterObj::State::OccupiedAtStart))
            active = true;

        if(active)
        {
            CableItemFlags dest;
            dest.cable.cable = mContacts.at(Contacts::PowerIn).cable;
            dest.cable.side = mContacts.at(Contacts::PowerIn).cableSide;
            dest.nodeContact = Contacts::PowerIn;
            dest.cable.pole = source.cable.pole;
            return {dest};
        }
    }

    // Make circuits end here
    return {};
}

QString TraintasticAxleCounterNode::nodeType() const
{
    return NodeType;
}

bool TraintasticAxleCounterNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    auto model = modeMgr()->modelForType(TraintasticAxleCounterObj::Type);
    if(model)
    {
        const QString objName = obj.value("axle_counter").toString();
        AbstractSimulationObject *activationObj = model->getObjectByName(objName);
        setAxleCounter(static_cast<TraintasticAxleCounterObj *>(activationObj));
    }
    else
        setAxleCounter(nullptr);

    return true;
}

void TraintasticAxleCounterNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["axle_counter"] = mAxleCounter ? mAxleCounter->name() : QString();
}

void TraintasticAxleCounterNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty objProp;
    objProp.name = "object";
    objProp.prettyName = tr("Object");
    objProp.types = {TraintasticAxleCounterObj::Type};
    result.append(objProp);
}

TraintasticAxleCounterObj *TraintasticAxleCounterNode::axleCounter() const
{
    return mAxleCounter;
}

bool TraintasticAxleCounterNode::setAxleCounter(TraintasticAxleCounterObj *newAxleCounter, bool steal)
{
    if(mAxleCounter == newAxleCounter)
        return true;

    if(!steal && newAxleCounter && newAxleCounter->hasContactNode())
        return false; // New axle counter is already powered

    if(mAxleCounter)
    {
        TraintasticAxleCounterObj *oldAxleCounter = mAxleCounter;
        oldAxleCounter->setContactNode(nullptr);
    }

    mAxleCounter = newAxleCounter;

    if(mAxleCounter)
    {
        mAxleCounter->setContactNode(this);
    }

    updateState(true);
    updateInputState();

    emit shapeChanged();
    modeMgr()->setFileEdited();

    return true;
}

void TraintasticAxleCounterNode::updateState(bool circuitChange)
{
    // Trigger graph update
    emit circuitsChanged();

    if(!circuitChange)
        return;

    if(mAxleCounter && mAxleCounter->state() == TraintasticAxleCounterObj::State::Free)
    {
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }
    else
    {
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this, Contacts::FreeTrackOut);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this, Contacts::FreeTrackOut);
    }

    if(mAxleCounter &&
            (mAxleCounter->state() == TraintasticAxleCounterObj::State::Occupied ||
             mAxleCounter->state() == TraintasticAxleCounterObj::State::OccupiedAtStart))
    {
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }
    else
    {
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this, Contacts::OccupiedTrackOut);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this, Contacts::OccupiedTrackOut);
    }

    if(mAxleCounter)
    {
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }
    else
    {
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this, Contacts::ResetIn);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this, Contacts::ResetIn);
    }
}

void TraintasticAxleCounterNode::addCircuit(ElectricCircuit *circuit)
{
    AbstractCircuitNode::addCircuit(circuit);
    updateInputState();
}

void TraintasticAxleCounterNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    AbstractCircuitNode::removeCircuit(circuit, items);
    updateInputState();
}

void TraintasticAxleCounterNode::partialRemoveCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    AbstractCircuitNode::partialRemoveCircuit(circuit, items);
    updateInputState();
}

void TraintasticAxleCounterNode::updateInputState()
{
    if(!mAxleCounter)
        return;

    const bool reset = hasCircuit(Contacts::ResetIn, CircuitType::Closed);
    const AnyCircuitType power = hasAnyCircuit(Contacts::PowerIn);
    TraintasticAxleCounterObj *axleCounter = mAxleCounter;
    QMetaObject::invokeMethod(axleCounter, [axleCounter, reset, power]()
                              {
                                  axleCounter->setHasPower(power != AnyCircuitType::None);
                                  axleCounter->triggerReset(reset);
                              }, Qt::QueuedConnection);
}
