/**
 * src/objects/interfaces/mechanicalinterface.h
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

#ifndef MECHANICALINTERFACE_H
#define MECHANICALINTERFACE_H

#include "abstractobjectinterface.h"

#include "../../utils/enum_desc.h"

#include <QVector>

class MechanicalInterface : public AbstractObjectInterface
{
public:
    typedef std::pair<int, int> LockRange;
    typedef QVector<LockRange> LockRanges;

    struct LockConstraint
    {
        AbstractSimulationObject *obj;
        LockRanges ranges;
    };
    typedef QVector<LockConstraint> LockConstraints;

    // Property names
    static constexpr QLatin1String LockRangePropName = QLatin1String("lock_range");
    static constexpr QLatin1String PositionPropName = QLatin1String("position");
    static constexpr QLatin1String AbsoluteRangePropName = QLatin1String("abs_range");


    MechanicalInterface(const EnumDesc &posDesc,
                        AbstractSimulationObject *obj);

    static constexpr QLatin1String IfaceType = QLatin1String("mechanical");
    QString ifaceType() override;

    bool loadFromJSON(const QJsonObject &obj) override;
    void saveToJSON(QJsonObject &obj) const override;

    void init();

    // State
    int position() const;
    void setPosition(int newPosition);

    inline const EnumDesc& positionDesc() const
    {
        return mPositionDesc;
    }

    int lockedMin() const;
    int lockedMax() const;

    // Options
    int absoluteMin() const;
    int absoluteMax() const;
    void setAbsoluteRange(int newMin, int newMax);

    inline LockConstraints constraints() const
    {
        return mConstraints;
    }

    void setObjectLockConstraints(AbstractSimulationObject *obj,
                                  const LockRanges& ranges);

    // Helpers
    LockRange getLockRangeForPos(int pos, int min, int max) const;

    void setLockedRange(int newMin, int newMax);
    void checkPositionValidForLock();

protected:
    inline bool isPositionValidForLock(int pos) const
    {
        return pos >= mLockedMin && pos <= mLockedMax;
    }

private:
    const EnumDesc mPositionDesc;
    LockConstraints mConstraints;

    int mPosition = 0;

    int mAbsoluteMin = 0;
    int mAbsoluteMax = 0;
    int mLockedMin = 0;
    int mLockedMax = 0;
};

#endif // MECHANICALINTERFACE_H
