/**
 * src/nodes/aceibuttonnode.cpp
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

#include "aceibuttonnode.h"

#include "../core/electriccircuit.h"

#include <QJsonObject>

ACEIButtonNode::ACEIButtonNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 3 sides
    mContacts.append(NodeContact("11", "12")); // Common
    mContacts.append(NodeContact("21", "22")); // Pressed
    mContacts.append(NodeContact("31", "32")); // Normal
}

ACEIButtonNode::~ACEIButtonNode()
{

}

QVector<CableItem> ACEIButtonNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact >= getContactCount()))
        return {};

    int otherContactIdx = -1;
    int otherContactIdx2 = -1;

    const NodeContact& sourceContact = mContacts.at(source.nodeContact);
    if(sourceContact.getType(source.cable.pole) == ContactType::Passthrough &&
            (source.nodeContact == 0 || source.nodeContact == 2))
    {
        // Pass to other contact straight
        otherContactIdx = source.nodeContact == 0 ? 2 : 0;
    }
    else if(mState != State::Extracted)
    {
        switch (source.nodeContact)
        {
        case 0:
            otherContactIdx = 2; // Normal is always connected
            if(mState == State::Pressed)
                otherContactIdx2 = 1;
            break;
        case 1:
            if(mState == State::Pressed)
            {
                otherContactIdx = 0;
                otherContactIdx2 = 2;
            }
            break;
        case 2:
            otherContactIdx = 0; // Common is always connected
            if(mState == State::Pressed)
                otherContactIdx2 = 1;
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

bool ACEIButtonNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    setFlipContact(obj.value("flip").toBool());

    return true;
}

void ACEIButtonNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["flip"] = flipContact();
}

QString ACEIButtonNode::nodeType() const
{
    return NodeType;
}

bool ACEIButtonNode::flipContact() const
{
    return mFlipContact;
}

void ACEIButtonNode::setFlipContact(bool newFlipContact)
{
    if(mFlipContact == newFlipContact)
        return;
    mFlipContact = newFlipContact;
    emit shapeChanged();
}

ACEIButtonNode::State ACEIButtonNode::state() const
{
    return mState;
}

void ACEIButtonNode::setState(State newState)
{
    if (mState == newState)
        return;

    State oldState = mState;
    mState = newState;
    emit stateChanged();

    bool hadCircuits = hasCircuits(CircuitType::Closed) || hasCircuits(CircuitType::Open);

    if(mState != State::Pressed)
    {
        // If pressed all circuits are enabled
        // Else delete old circuits
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        if(mState == State::Extracted)
            disableCircuits(closedCopy, this); // Disable all
        else
            disableCircuits(closedCopy, this, 1); // Only pressed circuits

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        if(mState == State::Extracted)
            truncateCircuits(openCopy, this); // Disable all
        else
            truncateCircuits(openCopy, this, 1); // Only pressed circuits

    }

    if(mState != State::Extracted)
    {
        // Scan for new circuits
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }

    if(hadCircuits)
    {
        ElectricCircuit::defaultReachNextOpenCircuit(this);
    }
}
