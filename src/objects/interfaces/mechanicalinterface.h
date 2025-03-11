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
#include "mechanical/mechanicalcondition.h"

#include "../../utils/enum_desc.h"

#include <QVector>
#include <QVarLengthArray>

class MechanicalInterface : public AbstractObjectInterface
{
public:
    typedef MechanicalCondition::LockRange LockRange;
    typedef MechanicalCondition::LockRanges LockRanges;
    typedef MechanicalCondition::LockConstraint LockConstraint;
    typedef MechanicalCondition::LockConstraints LockConstraints;
    typedef QVarLengthArray<MechanicalCondition::Type, 3> AllowedConditions;
    typedef QVarLengthArray<int, 3> LockablePositions;

    struct ConditionItem
    {
        MechanicalConditionSet conditions;
        QVector<LockConstraint> lockedObjects;
        QString title;

        inline bool isLocked() const
        {
            return !lockedObjects.isEmpty();
        }

        void setLocked(bool lock, AbstractSimulationObject *self);

        inline bool isSatisfied() const
        {
            return conditions.isSatisfied();
        }

        inline bool shouldLock(int position) const
        {
            return conditions.shouldLock(position);
        }
    };

    // Property names
    static constexpr QLatin1String LockRangePropName = QLatin1String("lock_range");
    static constexpr QLatin1String PositionPropName = QLatin1String("position");
    static constexpr QLatin1String AbsoluteRangePropName = QLatin1String("abs_range");
    static constexpr QLatin1String MecConditionsPropName = QLatin1String("mec_conditions");


    MechanicalInterface(const EnumDesc &posDesc,
                        AbstractSimulationObject *obj);
    ~MechanicalInterface();

    static constexpr QLatin1String IfaceType = QLatin1String("mechanical");
    QString ifaceType() override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    void getReferencedObjects(QSet<AbstractSimulationObject *> &result) override;

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
    LockRange getCurrentLockRange() const;

    void setLockedRange(int newMin, int newMax);
    void checkPositionValidForLock();

    inline int getConditionsSetsCount() const { return mConditionSets.size(); }

    void addConditionSet(const QString &title);
    void removeConditionSet(int idx);

    void setConditionSetRange(int idx, const LockRange& range);

    void setConditionSetConditions(int idx, const MechanicalCondition& c);
    ConditionItem getConditionSet(int idx) const;

    void recalculateWantedConditionState();

    const AllowedConditions& allowedConditionTypes() const;
    void setAllowedConditionTypes(const AllowedConditions &newAllowedConditions);

    const LockablePositions& lockablePositions() const;
    void setLockablePositions(const LockablePositions &newLockablePositions);

    inline bool userCanChangeAbsoulteRange() const
    {
        return mUserCanChangeAbsoulteRange;
    }

    inline void setUserCanChangeAbsoulteRange(bool val)
    {
        mUserCanChangeAbsoulteRange = val;
    }

protected:
    inline bool isPositionValidForLock(int pos) const
    {
        return pos >= mLockedMin && pos <= mLockedMax;
    }

    void recalculateObjectRelationship();

    void updateWantsLocks();

private:
    void registerRelationship(MechanicalInterface *other);
    void unregisterRelationship(MechanicalInterface *other, bool doRelock = true);

private:
    const EnumDesc mPositionDesc;
    LockConstraints mConstraints;
    LockRange mAllowedRangeByWanted;

    int mPosition = 0;

    int mAbsoluteMin = 0;
    int mAbsoluteMax = 0;
    int mLockedMin = 0;
    int mLockedMax = 0;

    bool mUserCanChangeAbsoulteRange = true;

    QVector<ConditionItem> mConditionSets;
    QVector<MechanicalInterface *> mWantsObjects;
    QVector<MechanicalInterface *> mWantedByObjects;

    AllowedConditions mAllowedConditionTypes;
    LockablePositions mLockablePositions;
};

#endif // MECHANICALINTERFACE_H
