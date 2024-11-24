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

#include <QJsonObject>

static inline bool contains(const MechanicalCondition::LockRange &range, int position)
{
    return range.first <= position && range.second >= position;
}

MechanicalInterface::MechanicalInterface(const EnumDesc &posDesc,
                                         AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
    , mPositionDesc(posDesc)
{

}

QString MechanicalInterface::ifaceType()
{
    return IfaceType;
}

bool MechanicalInterface::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractObjectInterface::loadFromJSON(obj))
        return false;

    // Range
    int pos_min = obj.value("pos_min").toInt();
    int pos_max = obj.value("pos_max").toInt();
    setAbsoluteRange(pos_min, pos_max);

    return true;
}

void MechanicalInterface::saveToJSON(QJsonObject &obj) const
{
    AbstractObjectInterface::saveToJSON(obj);

    // Range
    obj["pos_min"] = mAbsoluteMin;
    obj["pos_max"] = mAbsoluteMax;
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
            if(contains(r, position()))
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

void MechanicalInterface::addConditionSet(const MechanicalConditionSet &cond)
{
    ConditionItem item;
    item.conditions = cond;
    mConditionSets.append(item);

    recalculateObjectRelationship();
    recalculateWantedConditionState();
    updateWantsLocks();
}

void MechanicalInterface::recalculateWantedConditionState()
{
    LockRange allowedRange = {absoluteMin(), absoluteMax()};

    // Intersect all conditions' allowed ranges
    for(const ConditionItem& item : std::as_const(mConditionSets))
    {
        if(!item.conditions.isSatisfied())
        {
            // Condition's range applies only when it is not satisfied
            const LockRange subRange = item.conditions.allowedRangeWhenLocked;
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

    for(auto iface : newWants)
    {
        if(mWantsObjects.contains(iface))
            continue;

        // Add relationship to other object
        registerRelationship(iface);
    }

    for(auto iface : mWantsObjects)
    {
        if(newWants.contains(iface))
            continue;

        // Remove relationship to other object
        unregisterRelationship(iface);
    }

    // TODO: is this correct?
    //emitChanged(LockRangePropName, QVariant());
}

void MechanicalInterface::updateWantsLocks()
{
    // Notify all related objects want us in specific position to unlock
    for(MechanicalInterface *other : std::as_const(mWantedByObjects))
    {
        other->recalculateWantedConditionState();
    }

    for(ConditionItem& item : mConditionSets)
    {
        if(item.conditions.isSatisfied() &&
                !contains(item.conditions.allowedRangeWhenLocked, position()))
        {
            // We movede this object positions in a way that is allowed when this
            // condition is satisfied and locked. So now lock other objets.
            if(!item.lockedObjects.isEmpty())
                continue; // Already locked

            // Lock wanted objects
            item.conditions.rootCondition.getLockConstraints(item.lockedObjects);
            for(const LockConstraint& c : std::as_const(item.lockedObjects))
            {
                auto otherIface = c.obj->getInterface<MechanicalInterface>();
                Q_ASSERT(otherIface);

                // Lock other object
                otherIface->setObjectLockConstraints(object(), c.ranges);
            }
        }
        else
        {
            // We are inside allowed range so we can unlock other objects
            if(item.lockedObjects.isEmpty())
                continue; // Already unlocked

            // Unlock
            for(const LockConstraint& c : std::as_const(item.lockedObjects))
            {
                auto otherIface = c.obj->getInterface<MechanicalInterface>();
                Q_ASSERT(otherIface);

                // Reset our lock on other object
                otherIface->setObjectLockConstraints(object(), {});
            }
            item.lockedObjects.clear();
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

void MechanicalInterface::unregisterRelationship(MechanicalInterface *other)
{
    Q_ASSERT(mWantsObjects.contains(other));
    Q_ASSERT(other->mWantedByObjects.contains(this));

    mWantsObjects.removeOne(other);
    other->mWantedByObjects.removeOne(this);
}

bool MechanicalCondition::isSatisfied() const
{
    if(type == Type::Or)
    {
        return std::any_of(subConditions.constBegin(),
                           subConditions.constEnd(),
                           [](const MechanicalCondition& sub) -> bool
        {
            return sub.isSatisfied();
        });
    }
    else if(type == Type::And)
    {
        return std::all_of(subConditions.constBegin(),
                           subConditions.constEnd(),
                           [](const MechanicalCondition& sub) -> bool
        {
            return sub.isSatisfied();
        });
    }

    // Now check our lever
    if(!otherIface)
        return false;

    const int otherPosition = otherIface->position();

    if(type == Type::ExactPos)
        return otherPosition == requiredPositions.first;

    if(type == Type::RangePos)
        return contains(requiredPositions, otherPosition);

    if(type == Type::NotPos)
    {
        // This in reality is a range
        // The lever must not be on this side respect to its central position.
        // We use first position to deduce not allowed side
        const int centralPos = otherIface->positionDesc().defaultValue;
        const int notInPos = requiredPositions.first;
        if(notInPos < centralPos)
            return otherPosition >= centralPos;
        if(notInPos > centralPos)
            return otherPosition <= centralPos;
    }

    return false;
}

void MechanicalCondition::getAllObjects(QVector<MechanicalInterface *> &result) const
{
    if(type == Type::Or || type == Type::And)
    {
        for(auto sub : subConditions)
            sub.getAllObjects(result);
        return;
    }

    if(otherIface)
        result.append(otherIface);
}

void MechanicalCondition::getLockConstraints(LockConstraints &result) const
{
    if(!isSatisfied())
        return; // Only lock when satisfied

    if(type == Type::Or || type == Type::And)
    {
        for(auto sub : subConditions)
            sub.getLockConstraints(result);
        return;
    }

    // Now check our lever
    if(!otherIface)
        return;

    LockConstraint constraint;
    constraint.obj = otherIface->object();

    if(type == Type::ExactPos)
    {
        const int exactPos = requiredPositions.first;
        constraint.ranges.append({exactPos, exactPos});
    }
    else if(type == Type::RangePos)
    {
        constraint.ranges.append(requiredPositions);
    }
    else if(type == Type::NotPos)
    {
        // This in reality is a range
        // The lever must not be on this side respect to its central position.
        // We use first position to deduce not allowed side
        const int centralPos = otherIface->positionDesc().defaultValue;
        const int notInPos = requiredPositions.first;

        LockRange allowedRange = {otherIface->absoluteMin(),
                                  otherIface->absoluteMax()};
        if(notInPos < centralPos)
            allowedRange.first = centralPos;
        if(notInPos > centralPos)
            allowedRange.second = centralPos;

        constraint.ranges.append(allowedRange);
    }

    result.append(constraint);
}

bool MechanicalConditionSet::isSatisfied() const
{
    if(rootCondition.type == MechanicalCondition::Type::ExactPos && !rootCondition.otherIface)
    {
        // Empty condition, do not block
        return true;
    }

    return rootCondition.isSatisfied();
}
