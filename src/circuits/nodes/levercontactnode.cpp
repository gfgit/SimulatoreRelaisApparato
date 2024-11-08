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

#include "../../objects/lever/model/genericleverobject.h"
#include "../../objects/lever/model/genericlevermodel.h"

#include <QJsonObject>
#include "../../utils/genericleverutils.h"

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

bool LeverContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString leverName = obj.value("lever").toString();
    setLever(modeMgr()->leversModel()->getLever(leverName));

    setFlipContact(obj.value("flip").toBool());
    setSwapContactState(obj.value("swap_state").toBool());
    setHasCentralConnector(obj.value("central_connector").toBool(true));

    auto conditions = GenericLeverUtils::fromJSON(obj.value("conditions").toObject());
    setConditionSet(conditions);

    return true;
}

void LeverContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["lever"] = mLever ? mLever->name() : QString();
    obj["flip"] = flipContact();
    obj["swap_state"] = swapContactState();
    obj["central_connector"] = hasCentralConnector();

    obj["conditions"] = GenericLeverUtils::toJSON(mConditionSet);
}

QString LeverContactNode::nodeType() const
{
    return NodeType;
}

GenericLeverObject *LeverContactNode::lever() const
{
    return mLever;
}

void LeverContactNode::setLever(GenericLeverObject *newLever)
{
    if (mLever == newLever)
        return;

    if(mLever)
    {
        disconnect(mLever, &GenericLeverObject::positionChanged,
                   this, &LeverContactNode::onLeverPositionChanged);

        mLever->removeContactNode(this);
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &GenericLeverObject::positionChanged,
                this, &LeverContactNode::onLeverPositionChanged);

        mLever->addContactNode(this);
    }

    emit leverChanged(mLever);
    onLeverPositionChanged();
}

void LeverContactNode::onLeverPositionChanged()
{
    State s = State::Middle;

    if(mLever)
    {
        s = State::Up;

        // TODO: there is no middle transitions
        if(isPositionOn(int(mLever->position())))
            s = State::Down;
    }

    setState(s);

    // Refresh every lever position change
    emit stateChanged();
}

bool LeverContactNode::isPositionOn(int pos) const
{
    for(const LeverPositionCondition& item : std::as_const(mConditionSet))
    {
        if(item.type == LeverPositionConditionType::Exact)
        {
            if(item.positionFrom == pos)
                return true;
            continue;
        }

        // From/To
        if(item.positionFrom <= pos && pos <= item.positionTo)
            return true;
    }

    return false;
}

LeverContactNode::State LeverContactNode::state() const
{
    return mState;
}

void LeverContactNode::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;
    emit stateChanged();

    bool hadCircuits = hasCircuits(CircuitType::Closed) || hasCircuits(CircuitType::Open);

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

    if(hadCircuits)
    {
        ElectricCircuit::defaultReachNextOpenCircuit(this);
    }
}

LeverPositionConditionSet LeverContactNode::conditionSet() const
{
    return mConditionSet;
}

void LeverContactNode::setConditionSet(const LeverPositionConditionSet &newConditionSet)
{
    mConditionSet = newConditionSet;

    // Sanitize conditions
    for(auto it = mConditionSet.begin(); it != mConditionSet.end(); it++)
    {
        LeverPositionCondition& item = *it;
        if(item.type == LeverPositionConditionType::Exact)
            item.positionTo = item.positionFrom;
        else
        {
            item.positionTo = qMax(item.positionFrom + 2,
                                   item.positionTo);
        }
    }

    // Sort conditions
    std::sort(mConditionSet.begin(),
              mConditionSet.end(),
              [](const LeverPositionCondition& a,
              const LeverPositionCondition& b) -> bool
    {
        if(a.positionFrom == b.positionFrom)
        {
            if(a.type == b.type)
                return a.positionTo < b.positionTo;

            // Prefer ranges over exact positions
            return a.type > b.type;
        }
        return a.positionFrom < b.positionFrom;
    });

    // Remove duplicates/overlapping ranges
    int lastFromPosition = -1;
    int lastToPosition = -1;
    for(auto it = mConditionSet.begin(); it != mConditionSet.end();)
    {
        LeverPositionCondition& item = *it;
        if((item.positionFrom >= lastFromPosition && item.positionFrom <= lastToPosition)
                || (item.positionTo >= lastFromPosition && item.positionTo <= lastToPosition)
                || (item.positionFrom <= lastFromPosition && item.positionTo >= lastFromPosition)
                || (item.positionFrom <= lastToPosition && item.positionTo >= lastToPosition))
        {
            // Duplicates/overlapping
            it = mConditionSet.erase(it);
            continue;
        }

        lastFromPosition = item.positionFrom;
        lastToPosition = item.positionTo;
        it++;
    }

    // Notify settings changed
    modeMgr()->setFileEdited();

    // Refresh state based on new conditions
    onLeverPositionChanged();
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
    modeMgr()->setFileEdited();
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
    modeMgr()->setFileEdited();
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
    modeMgr()->setFileEdited();

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
