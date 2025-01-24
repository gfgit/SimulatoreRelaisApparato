/**
 * src/circuits/nodes/abstractdeviatornode.cpp
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

#include "abstractdeviatornode.h"

#include "../electriccircuit.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

AbstractDeviatorNode::AbstractDeviatorNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, false, parent}
{
    // 3 sides
    mContacts.append(NodeContact("11", "12")); // Common
    mContacts.append(NodeContact("21", "22")); // Up
    mContacts.append(NodeContact("31", "32")); // Down
}

QVector<CableItem> AbstractDeviatorNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact >= getContactCount()))
        return {};

    int otherContactIdx = -1;
    int otherContactIdx2 = -1;

    const NodeContact& sourceContact = mContacts.at(source.nodeContact);
    if(sourceContact.getType(source.cable.pole) == ContactType::Passthrough &&
            (source.nodeContact == CommonIdx || source.nodeContact == DownIdx))
    {
        // Pass to other contact straight
        otherContactIdx = source.nodeContact == CommonIdx ? DownIdx : CommonIdx;
    }
    else
    {
        switch (source.nodeContact)
        {
        case CommonIdx:
            if(isContactOn(UpIdx) && mHasCentralConnector)
                otherContactIdx = UpIdx;
            if(isContactOn(DownIdx))
                otherContactIdx2 = DownIdx;
            break;
        case UpIdx:
            if(isContactOn(UpIdx) && mHasCentralConnector)
            {
                otherContactIdx = CommonIdx;
                if(isContactOn(DownIdx))
                    otherContactIdx2 = DownIdx;
            }
            break;
        case DownIdx:
            if(isContactOn(DownIdx))
            {
                otherContactIdx = CommonIdx;
                if(isContactOn(UpIdx) && mHasCentralConnector)
                    otherContactIdx2 = UpIdx;
            }
            break;
        default:
            break;
        }
    }

    QVector<CableItem> result;

    if(otherContactIdx != -1)
    {
        const NodeContact& otherContact = mContacts.at(otherContactIdx);
        CableItem other;
        other.cable.cable = otherContact.cable;
        other.cable.side = otherContact.cableSide;
        other.cable.pole = source.cable.pole;
        other.nodeContact = otherContactIdx;

        result.append(other);
    }

    if(otherContactIdx2 != -1)
    {
        const NodeContact& otherContact = mContacts.at(otherContactIdx2);
        CableItem other;
        other.cable.cable = otherContact.cable;
        other.cable.side = otherContact.cableSide;
        other.cable.pole = source.cable.pole;
        other.nodeContact = otherContactIdx2;

        result.append(other);
    }

    return result;
}

bool AbstractDeviatorNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    setFlipContact(obj.value("flip").toBool());
    setSwapContactState(obj.value("swap_state").toBool());
    setHasCentralConnector(obj.value("central_connector").toBool(true));

    return true;
}

void AbstractDeviatorNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["flip"] = flipContact();
    obj["swap_state"] = swapContactState();
    obj["central_connector"] = hasCentralConnector();
}

bool AbstractDeviatorNode::swapContactState() const
{
    return mSwapContactState;
}

void AbstractDeviatorNode::setSwapContactState(bool newSwapContactState)
{
    if(!allowSwap())
        newSwapContactState = false;

    if(mSwapContactState == newSwapContactState)
        return;

    // Circuits must be disabled before editing contacts
    Q_ASSERT(getCircuits(CircuitType::Closed).isEmpty());
    Q_ASSERT(getCircuits(CircuitType::Open).isEmpty());

    mSwapContactState = newSwapContactState;

    // Swap current contact state
    std::swap(mContactOnArr[0], mContactOnArr[1]);

    emit shapeChanged();
    modeMgr()->setFileEdited();
}

bool AbstractDeviatorNode::flipContact() const
{
    return mFlipContact;
}

void AbstractDeviatorNode::setFlipContact(bool newFlipContact)
{
    if(mFlipContact == newFlipContact)
        return;

    mFlipContact = newFlipContact;

    emit shapeChanged();
    modeMgr()->setFileEdited();
}

bool AbstractDeviatorNode::hasCentralConnector() const
{
    return mHasCentralConnector;
}

void AbstractDeviatorNode::setHasCentralConnector(bool newHasCentralConnector)
{
    if(!mCanChangeCentralConnector)
        return;

    if(mHasCentralConnector == newHasCentralConnector)
        return;

    mHasCentralConnector = newHasCentralConnector;

    emit shapeChanged();
    modeMgr()->setFileEdited();

    if(!mHasCentralConnector)
    {
        // Circuits must be disabled before editing contacts
        Q_ASSERT(getCircuits(CircuitType::Closed).isEmpty());
        Q_ASSERT(getCircuits(CircuitType::Open).isEmpty());

        detachCable(1);
    }
}

void AbstractDeviatorNode::setContactState(bool valUp, bool valDown)
{
    if(mSwapContactState)
    {
        // Store as already swapped
        std::swap(valUp, valDown);
    }

    const bool hadCircuits = hasCircuits(CircuitType::Closed) || hasCircuits(CircuitType::Open);

    bool hasNewConnections = false;
    if(valUp && !isContactOn(UpIdx))
        hasNewConnections = true;
    if(valDown && !isContactOn(DownIdx))
        hasNewConnections = true;

    // Set new state
    mContactOnArr[0] = valUp;
    mContactOnArr[1] = valDown;

    // Remove existing circuits if no more connected
    // regardless of previous contact state
    if(!valUp)
    {
        if(!valDown)
        {
            // Remove all connections
            const CircuitList closedCopy = getCircuits(CircuitType::Closed);
            disableCircuits(closedCopy, this); // Disable all

            const CircuitList openCopy = getCircuits(CircuitType::Open);
            truncateCircuits(openCopy, this); // Disable all
        }
        else
        {
            // Remove up connections
            const CircuitList closedCopy = getCircuits(CircuitType::Closed);
            disableCircuits(closedCopy, this, 1); // Disable contact 1

            const CircuitList openCopy = getCircuits(CircuitType::Open);
            truncateCircuits(openCopy, this, 1); // Disable contact 1
        }
    }
    else if(!valDown)
    {
        // Remove down connections
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this, 2); // Disable contact 2

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this, 2); // Disable contact 2
    }

    {
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        for(ElectricCircuit *circuit : closedCopy)
        {
            Q_ASSERT(circuit->type() == CircuitType::Closed);
            Q_ASSERT(circuit->isEnabled());
        }

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        for(ElectricCircuit *circuit : openCopy)
        {
            Q_ASSERT(circuit->type() == CircuitType::Open);
            Q_ASSERT(circuit->isEnabled());
        }
    }

    if(hasNewConnections)
    {
        // Scan for new circuits
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }

    if(hadCircuits)
    {
        ElectricCircuit::defaultReachNextOpenCircuit(this);
    }

    emit deviatorStateChanged();
}

bool AbstractDeviatorNode::allowSwap() const
{
    return mAllowSwap;
}

void AbstractDeviatorNode::setAllowSwap(bool newAllowSwap)
{
    if(mAllowSwap == newAllowSwap)
        return;

    mAllowSwap = newAllowSwap;

    // Refresh swap
    setSwapContactState(swapContactState());

    // Always notify
    emit shapeChanged();
    modeMgr()->setFileEdited();
}

bool AbstractDeviatorNode::canChangeCentralConnector() const
{
    return mCanChangeCentralConnector;
}

void AbstractDeviatorNode::setCanChangeCentralConnector(bool newCanChangeCentralConnector)
{
    if(mCanChangeCentralConnector == newCanChangeCentralConnector)
        return;

    mCanChangeCentralConnector = newCanChangeCentralConnector;

    // Always notify
    emit shapeChanged();
    modeMgr()->setFileEdited();
}
