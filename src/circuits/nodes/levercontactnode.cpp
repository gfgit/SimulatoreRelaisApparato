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

//TODO: remove
#include "../../objects/lever/acei/aceileverobject.h"
#include "../../objects/lever/model/genericleverobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

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

    auto model = modeMgr()->modelForType(ACEILeverObject::Type);
    if(model)
    {
        const QString leverName = obj.value("lever").toString();
        AbstractSimulationObject *leverObj = model->getObjectByName(leverName);
        setLever(static_cast<GenericLeverObject *>(leverObj));
    }
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

    // Refresh at every lever position change
    emit deviatorStateChanged();
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

    setContactState(mState == State::Up,
                    mState == State::Down);
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
