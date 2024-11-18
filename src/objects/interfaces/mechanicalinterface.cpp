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

MechanicalInterface::MechanicalInterface(AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
{

}

QString MechanicalInterface::ifaceType()
{
    return IfaceType;
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
        return; // Do not add empty constraint

    // Add new ranges
    mConstraints.append(LockConstraint{obj, ranges});
    notifyObject();
}

bool MechanicalInterface::isPositionValid(int pos) const
{
    for(const LockConstraint &c : mConstraints)
    {
        bool allowed = false;

        // Do an OR on constraint's ranges
        for(const LockRange &r : c.ranges)
        {
            if(r.first <= pos && r.second >= pos)
            {
                allowed = true;
                break;
            }
        }

        // Do an AND on all constraints
        if(!allowed)
            return false;
    }

    return true;
}

MechanicalInterface::LockRange MechanicalInterface::getLockRangeForPos(int pos, int min, int max) const
{
    LockRange total = {min, max};

    for(const LockConstraint &c : mConstraints)
    {
        LockRange specific = {min, max};

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

        // Do an INTERSECTION on all constraints
        total.first = std::max(specific.first,
                               total.first);
        total.second = std::min(specific.second,
                                total.second);
    }

    return total;
}
