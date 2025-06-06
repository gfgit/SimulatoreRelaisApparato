/**
 * src/objects/interfaces/mechanical/mechanicalcondition.h
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

#ifndef MECHANICALCONDITION_H
#define MECHANICALCONDITION_H

#include <QVector>

class AbstractSimulationObject;
class MechanicalInterface;

class EnumDesc;

class ModeManager;
class QJsonObject;

struct MechanicalCondition
{
public:
    enum class Type
    {
        ExactPos = 0,
        RangePos,
        NotPos,
        Or,
        And
    };

    typedef std::pair<int, int> LockRange;
    typedef QVector<LockRange> LockRanges;

    struct LockConstraint
    {
        AbstractSimulationObject *obj;
        LockRanges ranges;
    };
    typedef QVector<LockConstraint> LockConstraints;

public:
    static const EnumDesc& getTypeDesc();

    static inline bool contains(const MechanicalCondition::LockRange &range, int position)
    {
        return range.first <= position && range.second >= position;
    }

public:
    bool isSatisfied() const;

    void getAllObjects(QVector<MechanicalInterface *>& result) const;

    void getLockConstraints(LockConstraints &result) const;

    bool containsReferencesTo(MechanicalInterface *iface) const;
    bool removeReferencesTo(MechanicalInterface *iface);

    bool removeInvalidConditions();

    void simplifyTree();

    static MechanicalCondition loadFromJSON(ModeManager *modeMgr, const QJsonObject& obj);
    void saveToJSON(QJsonObject& obj) const;

    inline bool operator==(const MechanicalCondition& other) const
    {
        return otherIface == other.otherIface &&
                requiredPositions == other.requiredPositions &&
                type == other.type &&
                subConditions == other.subConditions;
    }

    inline bool operator!=(const MechanicalCondition& other) const
    {
        return !(*this == other);
    }

    QString getHtmlString(bool parentSatisfied, bool topLevel) const;

public:
    MechanicalInterface *otherIface = nullptr;
    LockRange requiredPositions = {0, 0};
    Type type = Type::ExactPos;

    // For OR and AND
    QVector<MechanicalCondition> subConditions;
};

struct MechanicalConditionSet
{
    MechanicalCondition::LockRange allowedRangeWhenUnstatisfied;
    MechanicalCondition rootCondition;

    bool isSatisfied() const;

    bool shouldLock(int position) const;

    QString getHtmlString() const;
};

#endif // MECHANICALCONDITION_H
