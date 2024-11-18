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

MechanicalInterface::LockRange MechanicalInterface::getLockRangeForPos(int pos, int min, int max) const
{
    LockRange total = {min, max};

    for(const LockConstraint &c : mConstraints)
    {
        LockRange specific = {max, min}; // Reversed on purpose

        // Do an UNION on constraint's ranges
        for(const LockRange &r : c.ranges)
        {
            if(r.first <= pos && r.second >= pos)
            {
                specific.first = std::min(specific.first,
                                          r.first);
                specific.second = std::max(specific.second,
                                           r.second);
                break;
            }
        }

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
