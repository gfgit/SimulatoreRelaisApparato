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

#include "../../views/modemanager.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../objects/interfaces/leverinterface.h"

#include <QJsonObject>
#include "../../utils/genericleverutils.h"

LeverContactNode::LeverContactNode(ModeManager *mgr, QObject *parent)
    : AbstractDeviatorNode{mgr, parent}
{

}

LeverContactNode::~LeverContactNode()
{
    setLever(nullptr);
}

bool LeverContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractDeviatorNode::loadFromJSON(obj))
        return false;

    const QString leverName = obj.value("lever").toString();
    const QString leverType = obj.value("lever_type").toString();
    auto model = modeMgr()->modelForType(leverType);

    if(model)
        setLever(model->getObjectByName(leverName));
    else
        setLever(nullptr);

    auto conditions = GenericLeverUtils::fromJSON(obj.value("conditions").toObject());
    setConditionSet(conditions);

    return true;
}

void LeverContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);

    obj["lever"] = mLever ? mLever->name() : QString();
    obj["lever_type"] = mLever ? mLever->getType() : QString();

    obj["conditions"] = GenericLeverUtils::toJSON(mConditionSet);
}

void LeverContactNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty leverProp;
    leverProp.name = "lever";
    leverProp.prettyName = tr("Lever");
    leverProp.interface = LeverInterface::IfaceType;
    result.append(leverProp);
}

QString LeverContactNode::nodeType() const
{
    return NodeType;
}

AbstractSimulationObject *LeverContactNode::lever() const
{
    return mLever;
}

void LeverContactNode::setLever(AbstractSimulationObject *newLever)
{
    if (mLever == newLever)
        return;

    if(newLever && !newLever->getInterface<LeverInterface>())
        return;

    if(mLever)
    {
        disconnect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &LeverContactNode::onInterfacePropertyChanged);

        mLeverIface->removeContactNode(this);
        mLeverIface = nullptr;
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                this, &LeverContactNode::onInterfacePropertyChanged);

        mLeverIface = mLever->getInterface<LeverInterface>();
        mLeverIface->addContactNode(this);

        // Re-sanitize conditions based on new lever range
        setConditionSet(mConditionSet);
    }

    // TODO: sanitize conditions based on new lever type
    emit leverChanged(mLever);
    refreshContactState();
    modeMgr()->setFileEdited();
}

LeverInterface *LeverContactNode::leverIface() const
{
    return mLeverIface;
}

void LeverContactNode::onInterfacePropertyChanged(const QString& ifaceName,
                                              const QString& propName)
{
    if(ifaceName == LeverInterface::IfaceType &&
            propName == LeverInterface::PositionPropName)
    {
        refreshContactState();
    }
}

void LeverContactNode::refreshContactState()
{
    State s = State::Middle;

    bool isSpecialContact = false;
    if(mLever)
    {
        s = stateForPosition(mLeverIface->position(), isSpecialContact);
    }

    setState(s, isSpecialContact);

    // Refresh at every lever position change
    emit deviatorStateChanged();
}

LeverContactNode::State LeverContactNode::stateForPosition(int pos,
                                                           bool &specialContact) const
{
    // When conditions match, we are in Down state
    for(const LeverPositionCondition& item : std::as_const(mConditionSet))
    {
        if(item.type == LeverPositionConditionType::Exact)
        {
            if(item.positionFrom == pos)
            {
                specialContact = item.specialContact;
                return State::Down;
            }
            continue;
        }

        // From/To
        if(item.warpsAroundZero)
        {
            if(pos >= item.positionFrom || pos <= item.positionTo)
            {
                specialContact = item.specialContact;
                return State::Down;
            }
        }

        if(item.positionFrom <= pos && pos <= item.positionTo)
        {
            specialContact = item.specialContact;
            return State::Down;
        }
    }

    specialContact = false;
    return State::Up;
}

LeverContactNode::State LeverContactNode::state() const
{
    return mState;
}

void LeverContactNode::setState(State newState, bool specialContact)
{
    if (mState == newState)
        return;
    mState = newState;

    const bool wasSpecial = mIsSpecialContact;
    mIsSpecialContact = specialContact;

    setContactState(mState == State::Up,
                    mState == State::Down,
                    (wasSpecial || mIsSpecialContact));
}

LeverPositionConditionSet LeverContactNode::conditionSet() const
{
    return mConditionSet;
}

void LeverContactNode::setConditionSet(const LeverPositionConditionSet &newConditionSet)
{
    mConditionSet = newConditionSet;

    if(mLeverIface)
    {
        const auto posDesc = mLeverIface->positionDesc();

        // Sanitize conditions
        for(auto it = mConditionSet.begin(); it != mConditionSet.end(); it++)
        {
            it->positionFrom = std::clamp(it->positionFrom, posDesc.minValue, posDesc.maxValue);
            it->positionTo = std::clamp(it->positionTo, posDesc.minValue, posDesc.maxValue);
        }
    }

    // Sanitize conditions
    for(auto it = mConditionSet.begin(); it != mConditionSet.end(); it++)
    {
        LeverPositionCondition& item = *it;
        if(item.type == LeverPositionConditionType::Exact)
        {
            item.positionTo = item.positionFrom;
            item.warpsAroundZero = false;
        }
        else
        {
            if(mLeverIface && !mLeverIface->canWarpAroundZero())
                item.warpsAroundZero = false;

            if(!item.warpsAroundZero)
            {
                item.positionTo = qMax(item.positionFrom + 2,
                                       item.positionTo);
            }
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
    refreshContactState();
}
