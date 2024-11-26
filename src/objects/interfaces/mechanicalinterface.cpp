/**
 * src/objects/interfaces/mechanicalinterface.cpp
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

#include "mechanicalinterface.h"
#include "../abstractsimulationobject.h"
#include "../abstractsimulationobjectmodel.h"

#include <QJsonObject>
#include <QJsonArray>

MechanicalInterface::MechanicalInterface(const EnumDesc &posDesc,
                                         AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
    , mPositionDesc(posDesc)
{

}

MechanicalInterface::~MechanicalInterface()
{
    // Remove relationship to other object
    const auto wantsCopy = mWantsObjects;
    for(auto otherIface : wantsCopy)
    {
        unregisterRelationship(otherIface);
    }

    const auto wantedByCopy = mWantedByObjects;
    for(auto otherIface : wantedByCopy)
    {
        otherIface->unregisterRelationship(this);
    }
}

QString MechanicalInterface::ifaceType()
{
    return IfaceType;
}

bool MechanicalInterface::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractObjectInterface::loadFromJSON(obj, phase))
        return false;

    if(phase == LoadPhase::Creation)
    {
        // Range
        int pos_min = obj.value("pos_min").toInt();
        int pos_max = obj.value("pos_max").toInt();
        setAbsoluteRange(pos_min, pos_max);
    }
    else
    {
        // All objects created, we can build relationships

        // Conditions
        int idx = 0;
        const QJsonArray conditionsArr = obj.value("conditions").toArray();
        for(const QJsonValue& v : conditionsArr)
        {
            if(idx >= mConditionSets.size())
                break;

            const QJsonObject conditionObj = v.toObject();

            MechanicalCondition c = MechanicalCondition::loadFromJSON(object()->model()->modeMgr(),
                                                                      conditionObj);
            c.removeInvalidConditions();
            c.simplifyTree();

            mConditionSets[idx].conditions.rootCondition = c;
            emitChanged(MecConditionsPropName, idx);

            idx++;
        }

        // Refresh conditions state
        recalculateObjectRelationship();
        recalculateWantedConditionState();
        checkPositionValidForLock();
        updateWantsLocks();
    }

    return true;
}

void MechanicalInterface::saveToJSON(QJsonObject &obj) const
{
    AbstractObjectInterface::saveToJSON(obj);

    // Range
    obj["pos_min"] = mAbsoluteMin;
    obj["pos_max"] = mAbsoluteMax;

    // Conditions
    QJsonArray conditionsArr;
    for(const ConditionItem& item : mConditionSets)
    {
        QJsonObject conditionObj;
        item.conditions.rootCondition.saveToJSON(conditionObj);

        conditionsArr.append(conditionObj);
    }

    obj["conditions"] = conditionsArr;
}

void MechanicalInterface::init()
{
    // Recalculate range
    setAbsoluteRange(mPositionDesc.minValue, mPositionDesc.maxValue);
    setPosition(mPositionDesc.defaultValue);
}

void MechanicalInterface::setObjectLockConstraints(AbstractSimulationObject *obj, const LockRanges &ranges)
{
    Q_ASSERT(obj);

    // Remove old constraint
    LockConstraints::iterator it =
            std::find_if(mConstraints.begin(), mConstraints.end(),
                         [obj](const LockConstraint& c) -> bool
    {
        return c.obj == obj;
    });

    if(it != mConstraints.end())
    {
        if(it->ranges == ranges)
            return; // No change

        // Remove old constraint
        mConstraints.erase(it);
    }

    if(ranges.isEmpty())
    {
        emitChanged(LockRangePropName, QVariant());
        return; // Do not add empty constraint
    }

    // Add new ranges
    mConstraints.append(LockConstraint{obj, ranges});
    emitChanged(LockRangePropName, QVariant());
}

MechanicalInterface::LockRange MechanicalInterface::getCurrentLockRange() const
{
    LockRange total = {absoluteMin(), absoluteMax()};

    for(const LockConstraint &c : mConstraints)
    {
        LockRange specific = {absoluteMax(), absoluteMin()}; // Reversed on purpose

        // Do an UNION on constraint's ranges
        bool found = false;
        for(const LockRange &r : c.ranges)
        {
            if(MechanicalCondition::contains(r, position()))
            {
                // Current position is inside range
                specific.first = std::min(specific.first,
                                          r.first);
                specific.second = std::max(specific.second,
                                           r.second);
                // TODO: ranges should not overlap so here we would end loop
                found = true;
            }
        }

        if(!found)
            continue;

        // Now ensure max is greater or equal to min
        // since it was reversed above
        specific.second = std::max(specific.first,
                                   specific.second);

        // Do an INTERSECTION on all constraints
        total.first = std::max(specific.first,
                               total.first);
        total.second = std::min(specific.second,
                                total.second);
    }

    // Do an INTERSECTION on unsatisfied wanted conditions
    total.first = std::max(mAllowedRangeByWanted.first,
                           total.first);
    total.second = std::min(mAllowedRangeByWanted.second,
                            total.second);

    return total;
}

int MechanicalInterface::position() const
{
    return mPosition;
}

void MechanicalInterface::setPosition(int newPosition)
{
    if(mPosition == newPosition)
        return;

    // Always ensure all positions are passed
    const int increment = (newPosition > mPosition) ? +1 : -1;

    int p = mPosition;
    while(p != newPosition)
    {
        p += increment;

        mPosition = p;

        updateWantsLocks();

        emitChanged(PositionPropName, mPosition);
        emit mObject->stateChanged(mObject);
    }
}

int MechanicalInterface::absoluteMin() const
{
    return mAbsoluteMin;
}

int MechanicalInterface::absoluteMax() const
{
    return mAbsoluteMax;
}

void MechanicalInterface::setAbsoluteRange(int newMin, int newMax)
{
    newMin = qMax(newMin, 0);
    newMax = qMin(newMax, mPositionDesc.maxValue);

    if(newMax < newMin)
        newMax = newMin;

    const bool rangeChanged = (mAbsoluteMin != newMin) || (mAbsoluteMax != newMax);
    mAbsoluteMin = newMin;
    mAbsoluteMax = newMax;

    if(rangeChanged)
    {
        // Set fake locked range to allow every position
        // It will be recalculated by object if needed
        setLockedRange(absoluteMin(), absoluteMax());
        mAllowedRangeByWanted = {absoluteMin(), absoluteMax()};

        // Recalculate angle and normal position
        emitChanged(AbsoluteRangePropName, QVariant());
        emit mObject->settingsChanged(mObject);
    }
}

int MechanicalInterface::lockedMin() const
{
    return mLockedMin;
}

int MechanicalInterface::lockedMax() const
{
    return mLockedMax;
}

void MechanicalInterface::setLockedRange(int newMin, int newMax)
{
    newMin = qMax(newMin, mAbsoluteMin);
    newMax = qMin(newMax, mAbsoluteMax);

    if(newMax < newMin)
        newMax = newMin;

    if(mLockedMin == newMin && mLockedMax == newMax)
        return;

    mLockedMin = newMin;
    mLockedMax = newMax;
}

void MechanicalInterface::checkPositionValidForLock()
{
    if(!isPositionValidForLock(position()))
    {
        // Current position is not valid anymore,
        // move lever to closest valid position.

        // Iterate from current position, up and down
        int diff = 1;
        while(true)
        {
            const int prevPos = position() - diff;
            const int nextPos = position() + diff;

            if(prevPos < absoluteMin() && nextPos > absoluteMax())
                break;

            if(prevPos >= absoluteMin() && isPositionValidForLock(prevPos))
            {
                setPosition(prevPos);
                break;
            }

            if(nextPos <= absoluteMax() && isPositionValidForLock(nextPos))
            {
                setPosition(nextPos);
                break;
            }

            diff++;
        }
    }
}

void MechanicalInterface::addConditionSet(const QString& title)
{
    ConditionItem item;
    item.title = title;
    mConditionSets.append(item);
}

void MechanicalInterface::removeConditionSet(int idx)
{
    if(idx < 0 || idx >= mConditionSets.size())
        return;

    // Unlock
    ConditionItem& item = mConditionSets[idx];
    if(item.isLocked())
        item.setLocked(false, object());

    // Remove
    mConditionSets.removeAt(idx);

    // Refresh state
    recalculateObjectRelationship();
    recalculateWantedConditionState();
}

void MechanicalInterface::setConditionSetRange(int idx, const LockRange &range)
{
    if(idx < 0 || idx >= mConditionSets.size())
        return;

    // Update
    ConditionItem& item = mConditionSets[idx];
    item.conditions.allowedRangeWhenUnstatisfied = range;

    // Refresh state
    recalculateObjectRelationship();
    recalculateWantedConditionState();
    checkPositionValidForLock();
    updateWantsLocks();

    emitChanged(MecConditionsPropName, idx);
    emit mObject->settingsChanged(mObject);
}

void MechanicalInterface::setConditionSetConditions(int idx, const MechanicalCondition &c)
{
    if(idx < 0 || idx >= mConditionSets.size())
        return;

    // Update
    ConditionItem& item = mConditionSets[idx];

    MechanicalCondition simplified = c;
    simplified.removeInvalidConditions();
    simplified.simplifyTree();
    item.conditions.rootCondition = simplified;

    // Refresh state
    recalculateObjectRelationship();
    recalculateWantedConditionState();
    checkPositionValidForLock();
    updateWantsLocks();

    emitChanged(MecConditionsPropName, idx);
    emit mObject->settingsChanged(mObject);
}

MechanicalInterface::ConditionItem MechanicalInterface::getConditionSet(int idx) const
{
    if(idx < 0 || idx >= mConditionSets.size())
        return {};
    return mConditionSets.at(idx);
}

void MechanicalInterface::recalculateWantedConditionState()
{
    LockRange allowedRange = {absoluteMin(), absoluteMax()};

    // Intersect all conditions' allowed ranges
    for(const ConditionItem& cond : std::as_const(mConditionSets))
    {
        if(!cond.isSatisfied())
        {
            // Condition's range applies only when it is not satisfied
            const LockRange subRange = cond.conditions.allowedRangeWhenUnstatisfied;
            if(subRange.first > allowedRange.first)
                allowedRange.first = subRange.first;
            if(subRange.second < allowedRange.second)
                allowedRange.second = subRange.second;
        }
    }

    if(mAllowedRangeByWanted == allowedRange)
        return; // No change

    mAllowedRangeByWanted = allowedRange;
    emitChanged(LockRangePropName, QVariant());
}

void MechanicalInterface::recalculateObjectRelationship()
{
    QVector<MechanicalInterface *> newWants;

    for(const ConditionItem& item : std::as_const(mConditionSets))
    {
        item.conditions.rootCondition.getAllObjects(newWants);
    }

    for(auto iface : mWantsObjects)
    {
        if(newWants.contains(iface))
            continue;

        // Remove relationship to other object
        // Do not recalculate conditions,
        // it will be done manually later.
        unregisterRelationship(iface, false);
    }

    for(auto iface : newWants)
    {
        if(mWantsObjects.contains(iface))
            continue;

        // Add relationship to other object
        registerRelationship(iface);
    }
}

void MechanicalInterface::updateWantsLocks()
{
    // Notify all related objects want us in specific position to unlock
    for(MechanicalInterface *other : std::as_const(mWantedByObjects))
    {
        other->recalculateWantedConditionState();
    }

    for(ConditionItem& cond : mConditionSets)
    {
        if(cond.isSatisfied() && cond.shouldLock(position()))
        {
            // We movede this object positions in a way that is allowed when this
            // condition is satisfied and locked. So now lock other objets.
            if(cond.isLocked())
                continue; // Already locked

            cond.setLocked(true, object());
        }
        else
        {
            // We are inside allowed range so we can unlock other objects
            if(!cond.isLocked())
                continue; // Already unlocked

            cond.setLocked(false, object());
        }
    }
}

void MechanicalInterface::registerRelationship(MechanicalInterface *other)
{
    Q_ASSERT(!mWantsObjects.contains(other));
    Q_ASSERT(!other->mWantedByObjects.contains(this));

    mWantsObjects.append(other);
    other->mWantedByObjects.append(this);
}

void MechanicalInterface::unregisterRelationship(MechanicalInterface *other, bool doRelock)
{
    Q_ASSERT(mWantsObjects.contains(other));
    Q_ASSERT(other->mWantedByObjects.contains(this));

    mWantsObjects.removeOne(other);
    other->mWantedByObjects.removeOne(this);

    // Remove ranges locked by us
    other->setObjectLockConstraints(object(), {});

    bool needsRelock = false;

    // Remove other object from our conditions
    for(ConditionItem& item : mConditionSets)
    {
        if(!item.conditions.rootCondition.containsReferencesTo(other))
            continue; // This condition does not reference other item, skip it

        if(item.isLocked())
        {
            item.setLocked(false, object()); // Release referenced objects
            needsRelock = true;
        }

        item.conditions.rootCondition.removeReferencesTo(other);
    }

    if(needsRelock && doRelock)
    {
        // Re-lock referenced objects if needed
        // Some conditions may not be satisfied anymore
        // so also recalculate their state
        recalculateWantedConditionState();
        updateWantsLocks();
    }
}

const MechanicalInterface::LockablePositions& MechanicalInterface::lockablePositions() const
{
    return mLockablePositions;
}

void MechanicalInterface::setLockablePositions(const LockablePositions &newLockablePositions)
{
    if(mLockablePositions == newLockablePositions)
        return;

    mLockablePositions = newLockablePositions;
}

const MechanicalInterface::AllowedConditions& MechanicalInterface::allowedConditionTypes() const
{
    return mAllowedConditionTypes;
}

void MechanicalInterface::setAllowedConditionTypes(const AllowedConditions &newAllowedConditions)
{
    if(mAllowedConditionTypes == newAllowedConditions)
        return;

    mAllowedConditionTypes = newAllowedConditions;
}
