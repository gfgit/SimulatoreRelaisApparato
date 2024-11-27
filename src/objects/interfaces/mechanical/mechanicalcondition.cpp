/**
 * src/objects/interfaces/mechanical/mechanicalcondition.cpp
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

#include "mechanicalcondition.h"
#include "../mechanicalinterface.h"

#include "../../abstractsimulationobject.h"

#include "../../../utils/enum_desc.h"

#include "../../../views/modemanager.h"
#include "../../abstractsimulationobjectmodel.h"
#include "../../abstractsimulationobject.h"

#include <QCoreApplication> // For Q_DECLARE_TR_FUNCTIONS()

#include <QJsonObject>
#include <QJsonArray>

class MechanicalConditionTypeTranslation
{
    Q_DECLARE_TR_FUNCTIONS(MechanicalConditionTypeTranslation)
};

static const EnumDesc mechanical_condition_type_desc =
{
    int(MechanicalCondition::Type::ExactPos),
    int(MechanicalCondition::Type::And),
    int(MechanicalCondition::Type::ExactPos),
    "MechanicalConditionTypeTranslation",
    {
        QT_TRANSLATE_NOOP("MechanicalConditionTypeTranslation", "Exact"),
        QT_TRANSLATE_NOOP("MechanicalConditionTypeTranslation", "Range"),
        QT_TRANSLATE_NOOP("MechanicalConditionTypeTranslation", "Not"),
        QT_TRANSLATE_NOOP("MechanicalConditionTypeTranslation", "OR"),
        QT_TRANSLATE_NOOP("MechanicalConditionTypeTranslation", "AND")
    }
};

const EnumDesc &MechanicalCondition::getTypeDesc()
{
    return mechanical_condition_type_desc;
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
        for(const auto &sub : subConditions)
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
        for(const auto &sub : subConditions)
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

bool MechanicalCondition::containsReferencesTo(MechanicalInterface *iface) const
{
    if(type == Type::Or || type == Type::And)
    {
        bool result = false;
        for(const auto& sub : subConditions)
            result |= sub.containsReferencesTo(iface);
        return result;
    }

    if(otherIface == iface)
    {
        return true;
    }

    return false;
}

bool MechanicalCondition::removeReferencesTo(MechanicalInterface *iface)
{
    if(type == Type::Or || type == Type::And)
    {
        bool result = false;
        for(auto &sub : subConditions)
            result |= sub.removeReferencesTo(iface);
        return result;
    }

    if(otherIface == iface)
    {
        // Reset our item
        *this = MechanicalCondition();

        // Item was reset
        return true;
    }

    return false;
}

bool MechanicalCondition::removeInvalidConditions()
{
    if(type == Type::Or || type == Type::And)
    {
        bool result = false;
        for(auto &sub : subConditions)
            result |= sub.removeInvalidConditions();
        return result;
    }

    if(!otherIface)
        return false;

    const int min = otherIface->absoluteMin();
    const int max = otherIface->absoluteMax();

    // Check position is in range
    bool valid = contains({min, max}, requiredPositions.first) &&
            otherIface->lockablePositions().contains(requiredPositions.first);

    // Second must be greated than or equal to first
    requiredPositions.second = std::max(requiredPositions.first,
                                        requiredPositions.second);

    // Check also second position for ranges
    if(type == Type::RangePos && valid)
        contains({min, max}, requiredPositions.second) &&
                otherIface->lockablePositions().contains(requiredPositions.second);

    if(valid)
    {
        // Check allowed types, this cannot be fixed
        valid = otherIface->allowedConditionTypes().contains(type);
    }

    if(!valid)
    {
        // Reset our item
        *this = MechanicalCondition();

        // Item was reset
        return true;
    }

    return false;
}

void MechanicalCondition::simplifyTree()
{
    if(type == Type::And || type == Type::Or)
    {
        for(auto it = subConditions.begin(); it != subConditions.end(); )
        {
            auto &sub = *it;
            sub.simplifyTree();

            if(sub.type == Type::And && type == Type::And)
            {
                sub.simplifyTree();

                auto beforeIt = it + 1;

                // Merge all sub ANDs togheter
                for(const auto& sub2 : sub.subConditions)
                {
                    beforeIt = subConditions.insert(beforeIt, sub2);
                    beforeIt++;
                }

                // Remove original sub item
                it = subConditions.erase(it);

                // Skip newly inserted items
                it += sub.subConditions.size();
                continue;
            }
            else
            {
                // Remove condition if empty
                if(sub == MechanicalCondition())
                {
                    it = subConditions.erase(it);
                    continue;
                }
            }

            it++;
        }

        if(subConditions.isEmpty())
        {
            // Reset this item
            *this = MechanicalCondition();
        }
        else if(subConditions.size() == 1)
        {
            // Replace with sub condition
            *this = subConditions.first();
        }
        return;
    }

    // Normal item
    if(!otherIface)
    {
        // Reset this item
        *this = MechanicalCondition();
    }
}

MechanicalCondition MechanicalCondition::loadFromJSON(ModeManager *modeMgr, const QJsonObject &obj)
{
    int type = getTypeDesc().valueForUntranslated(obj.value("type").toString().toLatin1());
    if(type < 0)
        return {};

    MechanicalCondition c;
    c.type = Type(type);

    if(c.type == Type::And || c.type == Type::Or)
    {
        // Load sub conditions
        const QJsonArray arr = obj.value("sub_conditions").toArray();
        c.subConditions.reserve(arr.size());

        for(const QJsonValue& v : arr)
        {
            const QJsonObject subObj = v.toObject();
            MechanicalCondition sub = loadFromJSON(modeMgr, subObj);
            c.subConditions.append(sub);
        }
    }
    else
    {
        const QString objName = obj.value("object").toString();
        const QString objType = obj.value("object_type").toString();
        auto model = modeMgr->modelForType(objType);

        if(model)
        {
            AbstractSimulationObject *object = model->getObjectByName(objName);
            if(object)
                c.otherIface = object->getInterface<MechanicalInterface>();
        }

        c.requiredPositions.first = obj.value("pos1").toInt();
        if(c.type == Type::RangePos)
            c.requiredPositions.second = obj.value("pos2").toInt();
        else
            c.requiredPositions.second = c.requiredPositions.first;
    }

    return c;
}

void MechanicalCondition::saveToJSON(QJsonObject &obj) const
{
    obj["type"] = QString::fromLatin1(getTypeDesc().untranslatedName(int(type)));

    if(type == Type::And || type == Type::Or)
    {
        // Store sub conditions
        QJsonArray arr;
        for(const MechanicalCondition& sub : subConditions)
        {
            QJsonObject subObj;
            sub.saveToJSON(subObj);
            arr.append(subObj);
        }

        obj["sub_conditions"] = arr;
    }
    else
    {
        obj["object"] = otherIface ? otherIface->object()->name() : QString();
        obj["object_type"] = otherIface ? otherIface->object()->getType() : QString();

        obj["pos1"] = requiredPositions.first;
        if(type == Type::RangePos)
            obj["pos2"] = requiredPositions.second;
    }
}

bool MechanicalConditionSet::isSatisfied() const
{
    if(rootCondition == MechanicalCondition())
    {
        // Empty condition, do not block
        return true;
    }

    return rootCondition.isSatisfied();
}

bool MechanicalConditionSet::shouldLock(int position) const
{
    // Lock when position is out of allowed range for unsatisfied position
    if(MechanicalCondition::contains(allowedRangeWhenUnstatisfied, position))
        return false;

    // We went out of allowed range with unstatisfied condition
    // so now condition must be satisfied (otherwise we would have
    // been constrained to allowedRangeWhenUnstatisfied)
    // and we now should apply this condition by locking it
    return true;
}

void MechanicalInterface::ConditionItem::setLocked(bool lock, AbstractSimulationObject *self)
{
    if(lock == isLocked())
        return;

    if(lock)
    {
        // Lock wanted objects
        conditions.rootCondition.getLockConstraints(lockedObjects);
        for(const LockConstraint& c : std::as_const(lockedObjects))
        {
            auto otherIface = c.obj->getInterface<MechanicalInterface>();
            Q_ASSERT(otherIface);

            // Lock other object
            otherIface->setObjectLockConstraints(self, c.ranges);
        }
    }
    else
    {
        // Unlock
        for(const LockConstraint& c : std::as_const(lockedObjects))
        {
            auto otherIface = c.obj->getInterface<MechanicalInterface>();
            Q_ASSERT(otherIface);

            // Reset our lock on other object
            otherIface->setObjectLockConstraints(self, {});
        }
        lockedObjects.clear();
    }
}
