/**
 * src/circuits/nodes/levercontactnode.cpp
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

#include "levercontactnode.h"

#include "../electriccircuit.h"

#include "../../views/modemanager.h"

#include "../../objects/acei_lever/model/aceileverobject.h"

#include <QJsonObject>

LeverContactNode::LeverContactNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, false, parent}
{
    // 3 sides
    mContacts.append(NodeContact("11", "12")); // Common
    mContacts.append(NodeContact("21", "22")); // Up
    mContacts.append(NodeContact("31", "32")); // Down
}

LeverContactNode::~LeverContactNode()
{
    setLever(nullptr);
}

QVector<CableItem> LeverContactNode::getActiveConnections(CableItem source, bool invertDir)
{
    // TODO: same as RelaisContactNode
    if((source.nodeContact < 0) || (source.nodeContact >= getContactCount()))
        return {};

    int otherContactIdx = -1;

    const NodeContact& sourceContact = mContacts.at(source.nodeContact);
    if(sourceContact.getType(source.cable.pole) == ContactType::Passthrough &&
            (source.nodeContact == 0 || source.nodeContact == 2))
    {
        // Pass to other contact straight
        otherContactIdx = source.nodeContact == 0 ? 2 : 0;
    }
    else
    {
        bool isDown = !m_isOn;
        if(swapContactState())
            isDown = !isDown;

        switch (source.nodeContact)
        {
        case 0:
            otherContactIdx = isDown ? 2 : 1;
            break;
        case 1:
            if(!isDown)
                otherContactIdx = 0;
            break;
        case 2:
            if(isDown)
                otherContactIdx = 0;
            break;
        default:
            break;
        }
    }

    if(otherContactIdx != -1)
    {
        const NodeContact& otherContact = mContacts.at(otherContactIdx);
        CableItem other;
        other.cable.cable = otherContact.cable;
        other.cable.side = otherContact.cableSide;
        other.cable.pole = source.cable.pole;
        other.nodeContact = otherContactIdx;

        return {other};
    }

    return {};
}

bool LeverContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString leverName = obj.value("lever").toString();
    //setLever(modeMgr()->relaisModel()->getRelay(relaisName));

    return true;
}

void LeverContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["lever"] = mLever ? mLever->name() : QString();
}

QString LeverContactNode::nodeType() const
{
    return NodeType;
}

ACEILeverObject *LeverContactNode::lever() const
{
    return mLever;
}

void LeverContactNode::setLever(ACEILeverObject *newLever)
{
    if (mLever == newLever)
        return;

    mLever = newLever;

    if(mLever)
    {
        disconnect(mLever, &ACEILeverObject::positionChanged,
                   this, &LeverContactNode::onLeverPositionChanged);

        //mLever->removeContactNode(this);
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &ACEILeverObject::positionChanged,
                this, &LeverContactNode::onLeverPositionChanged);

        //mLever->addContactNode(this);
    }

    emit leverChanged(mLever);
    onLeverPositionChanged();
}

bool LeverContactNode::isOn() const
{
    return m_isOn;
}

void LeverContactNode::setIsOn(bool newIsOn)
{
    if (m_isOn == newIsOn)
        return;
    m_isOn = newIsOn;
    emit isOnChanged(m_isOn);
}

void LeverContactNode::onLeverPositionChanged()
{
    if(modeMgr()->mode() == FileMode::Editing)
        return; // Prevent turning on during editing

    bool newOn = false;
    if(mLever)
        newOn = isPositionOn(mLever->position());

    if (m_isOn == newOn)
        return;
    m_isOn = newOn;
    emit isOnChanged(m_isOn);

    if(m_isOn)
    {
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }
    else
    {
        bool hadCircuits = hasCircuits(CircuitType::Closed) || hasCircuits(CircuitType::Open);

        // Disable circuits
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this);

        if(hadCircuits)
        {
            ElectricCircuit::defaultReachNextOpenCircuit(this);
        }
    }
}

bool LeverContactNode::isPositionOn(ACEILeverPosition pos) const
{
    ACEILeverPosition onPos1 = ACEILeverPosition::Left;

    ACEILeverPosition onPos2From = ACEILeverPosition::Normal;
    ACEILeverPosition onPos2To = ACEILeverPosition::Right;

    if(pos == onPos1)
        return true;

    if(pos >= onPos2From && pos <= onPos2To)
        return true;

    return false;
}

bool LeverContactNode::swapContactState() const
{
    return mSwapContactState;
}

void LeverContactNode::setSwapContactState(bool newSwapContactState)
{
    if(mSwapContactState == newSwapContactState)
        return;
    mSwapContactState = newSwapContactState;
    emit shapeChanged();
}

bool LeverContactNode::flipContact() const
{
    return mFlipContact;
}

void LeverContactNode::setFlipContact(bool newFlipContact)
{
    if(mFlipContact == newFlipContact)
        return;
    mFlipContact = newFlipContact;
    emit shapeChanged();
}

bool LeverContactNode::hasCentralConnector() const
{
    return mHasCentralConnector;
}

void LeverContactNode::setHasCentralConnector(bool newHasCentralConnector)
{
    if(mHasCentralConnector == newHasCentralConnector)
        return;
    mHasCentralConnector = newHasCentralConnector;
    emit shapeChanged();

    if(!mHasCentralConnector)
    {
        // Remove circuits and detach cable
        // No need to re-add circuits later
        // Since it will not have cable attached
        if(hasCircuit(1) > 0)
        {
            // Disable all circuits passing on disabled contact
            const CircuitList closedCopy = getCircuits(CircuitType::Closed);
            disableCircuits(closedCopy, this, 1);

            const CircuitList openCopy = getCircuits(CircuitType::Open);
            truncateCircuits(openCopy, this, 1);
        }

        detachCable(1);
    }
}
