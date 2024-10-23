/**
 * src/nodes/relaiscontactnode.cpp
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

#include "relaiscontactnode.h"

#include "../core/electriccircuit.h"
#include "../objects/abstractrelais.h"

#include "../objects/relaismodel.h"

#include <QJsonObject>

RelaisContactNode::RelaisContactNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 3 sides
    mContacts.append(NodeContact("11", "12")); // Common
    mContacts.append(NodeContact("21", "22")); // Up
    mContacts.append(NodeContact("31", "32")); // Down
}

RelaisContactNode::~RelaisContactNode()
{
    setRelais(nullptr);
}

QVector<CableItem> RelaisContactNode::getActiveConnections(CableItem source, bool invertDir)
{
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
    else if(mState != State::Middle)
    {
        bool isDown = mState == State::Down;
        if(swapContactState())
            isDown = mState == State::Up;

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

bool RelaisContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString relaisName = obj.value("relais").toString();

    setRelais(relaisModel()->getRelay(relaisName));

    setFlipContact(obj.value("flip").toBool());
    setSwapContactState(obj.value("swap_state").toBool());
    setHasCentralConnector(obj.value("central_connector").toBool(true));

    return true;
}

void RelaisContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();
    obj["flip"] = flipContact();
    obj["swap_state"] = swapContactState();
    obj["central_connector"] = hasCentralConnector();
}

QString RelaisContactNode::nodeType() const
{
    return NodeType;
}

AbstractRelais *RelaisContactNode::relais() const
{
    return mRelais;
}

void RelaisContactNode::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;

    if(mRelais)
    {
        disconnect(mRelais, &AbstractRelais::stateChanged,
                   this, &RelaisContactNode::onRelaisStateChanged);

        mRelais->removeContactNode(this);
    }

    mRelais = newRelais;

    if(mRelais)
    {
        connect(mRelais, &AbstractRelais::stateChanged,
                this, &RelaisContactNode::onRelaisStateChanged);

        mRelais->addContactNode(this);
    }

    emit relayChanged(mRelais);
    onRelaisStateChanged();
}

RelaisModel *RelaisContactNode::relaisModel() const
{
    return mRelaisModel;
}

void RelaisContactNode::setRelaisModel(RelaisModel *newRelaisModel)
{
    mRelaisModel = newRelaisModel;
}

bool RelaisContactNode::swapContactState() const
{
    return mSwapContactState;
}

void RelaisContactNode::setSwapContactState(bool newSwapContactState)
{
    if(mSwapContactState == newSwapContactState)
        return;
    mSwapContactState = newSwapContactState;
    emit shapeChanged();
}

bool RelaisContactNode::flipContact() const
{
    return mFlipContact;
}

void RelaisContactNode::setFlipContact(bool newFlipContact)
{
    if(mFlipContact == newFlipContact)
        return;
    mFlipContact = newFlipContact;
    emit shapeChanged();
}

bool RelaisContactNode::hasCentralConnector() const
{
    return mHasCentralConnector;
}

void RelaisContactNode::setHasCentralConnector(bool newHasCentralConnector)
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

RelaisContactNode::State RelaisContactNode::state() const
{
    return mState;
}

void RelaisContactNode::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;
    emit stateChanged();

    // Disable circuits
    const CircuitList closedCopy = getCircuits(CircuitType::Closed);
    disableCircuits(closedCopy, this);

    const CircuitList openCopy = getCircuits(CircuitType::Open);
    truncateCircuits(openCopy, this);

    if(mState != State::Middle)
    {
        // Scan for new circuits
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }
}

void RelaisContactNode::onRelaisStateChanged()
{
    State s = State::Middle;
    if(mRelais)
    {
        switch (mRelais->state())
        {
        case AbstractRelais::State::Down:
            s = State::Down;
            break;
        case AbstractRelais::State::Up:
            s = State::Up;
            break;
        default:
            break;
        }
    }

    setState(s);
}
