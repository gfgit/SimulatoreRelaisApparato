/**
 * src/objects/lever/ace_sasib/acesasiblever7positions.cpp
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

#include "acesasiblever7positions.h"

#include "../../interfaces/mechanicalinterface.h"
#include "../../interfaces/leverinterface.h"

#include "../../simple_activable/electromagnet.h"



#include "../../abstractsimulationobjectmodel.h"
#include "acesasiblever5positions.h"

#include "../../../views/modemanager.h"

static const EnumDesc ace_sasib_7_posDesc =
{
    int(ACESasibLeverPosition7::TurnedForward),
    int(ACESasibLeverPosition7::TurnedBackwards),
    int(ACESasibLeverPosition7::Normal),
    "ACESasibLever7PosObject",
    {
        QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Forward"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Liberation F"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Immobilization F"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Normal"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Immobilization B"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Liberation B"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Backwards")
    }
};

static const LeverAngleDesc ace_sasib_7_angleDesc =
{
    {-80, -135}, // Forward
    {}, // Middle1
    {-50, -90}, // Wait Liberation F
    {}, // Middle2
    {-30, -45}, // Wait Immobilization F
    {}, // Middle3
    {  0,   0}, // Normal
    {}, // Middle4
    {+30, +45}, // Wait Immobilization B
    {}, // Middle5
    {+50, +90}, // Wait Liberation B
    {}, // Middle6
    {+80, +135}, // Backwards
};

inline QLatin1String getPosSuffix(ACESasibLeverPosition7 pos)
{
    switch (pos)
    {
    case ACESasibLeverPosition7::TurnedForward:
    {
        return QLatin1String("a");
    }
    case ACESasibLeverPosition7::WaitLiberationForward:
    {
        return QLatin1String("la");
    }
    case ACESasibLeverPosition7::WaitLiberationBackwards:
    {
        return QLatin1String("li");
    }
    case ACESasibLeverPosition7::TurnedBackwards:
    {
        return QLatin1String("i");
    }
    default:
        break;
    }

    return QLatin1String();
}

static QString ACESasibLever7PosObject_getCondName(AbstractSimulationObject *obj,
                                                   MechanicalCondition::Type t,
                                                   const MechanicalCondition::LockRange& r)
{
    if(!obj)
        return QString();

    static const QLatin1String suffixFmt("<u>%1%2</u>");
    ACESasibLeverPosition7 pos = ACESasibLeverPosition7(r.first);

    switch (t)
    {
    case MechanicalCondition::Type::ExactPos:
    {
        if(pos == ACESasibLeverPosition7::Normal)
            return obj->name();

        QLatin1String suffix = getPosSuffix(pos);
        if(suffix.isEmpty())
            return QString();

        return suffixFmt.arg(obj->name(), suffix);
    }
    case MechanicalCondition::Type::RangePos:
    {
        ACESasibLeverPosition7 pos2 = ACESasibLeverPosition7(r.second);

        QString result;

        if(pos == ACESasibLeverPosition7::Normal)
            result = obj->name();
        else
        {
            QLatin1String suffix = getPosSuffix(pos);
            if(suffix.isEmpty())
                return QString();

            result = suffixFmt.arg(obj->name(), suffix);
        }

        result += QLatin1String("-");

        if(pos2 == ACESasibLeverPosition7::Normal)
            result += obj->name();
        else
        {
            QLatin1String suffix = getPosSuffix(pos2);
            if(suffix.isEmpty())
                return QString();

            result += suffixFmt.arg(obj->name(), suffix);
        }

        return result;
    }
    case MechanicalCondition::Type::NotPos:
    {
        if(pos == ACESasibLeverPosition7::Normal)
            return QString(); // Cannot be used in NOT conditions

        QLatin1String suffix = getPosSuffix(pos);
        if(suffix.isEmpty())
            return QString();

        // No underline in NOT
        return obj->name() + suffix;
    }
    default:
        break;
    }

    return QString();
}


ACESasibLever7PosObject::ACESasibLever7PosObject(AbstractSimulationObjectModel *m)
    : ACESasibLeverCommonObject(m, ace_sasib_7_posDesc, ace_sasib_7_angleDesc)
{
    mechanicalIface->setAllowedConditionTypes(
                {
                    MechanicalCondition::Type::ExactPos,
                    MechanicalCondition::Type::RangePos,
                    MechanicalCondition::Type::NotPos
                });
    mechanicalIface->setCondNameFunc(&ACESasibLever7PosObject_getCondName);

    mechanicalIface->setLockablePositions(
                {
                    int(ACESasibLeverPosition7::TurnedForward),
                    int(ACESasibLeverPosition7::WaitLiberationForward),
                    int(ACESasibLeverPosition7::Normal),
                    int(ACESasibLeverPosition7::WaitLiberationBackwards),
                    int(ACESasibLeverPosition7::TurnedBackwards)
                });

    mechanicalIface->addConditionSet(tr("Go Forward"));
    mechanicalIface->setConditionSetRange(0,
                                          {int(ACESasibLeverPosition7::Normal),
                                           int(ACESasibLeverPosition7::TurnedBackwards)});

    mechanicalIface->addConditionSet(tr("Go Backwards"));
    mechanicalIface->setConditionSetRange(1,
                                          {int(ACESasibLeverPosition7::TurnedForward),
                                           int(ACESasibLeverPosition7::Normal)});
}

QString ACESasibLever7PosObject::getType() const
{
    return Type;
}

void ACESasibLever7PosObject::addElectromagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition7 pos = ACESasibLeverPosition7(leverInterface->position());

    std::pair<ACESasibLeverPosition7, ACESasibLeverPosition7> range;
    bool empty = false;

    if(pos <= ACESasibLeverPosition7::WaitLiberationForward)
    {
        range = {ACESasibLeverPosition7::TurnedForward,
                 ACESasibLeverPosition7::WaitLiberationForward};
    }
    else if(pos > ACESasibLeverPosition7::WaitLiberationForward &&
            pos < ACESasibLeverPosition7::WaitImmobilizationForward)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition7::WaitImmobilizationForward &&
            pos <= ACESasibLeverPosition7::WaitImmobilizationBackwards)
    {
        // There is Normal position inside this range
        range = {ACESasibLeverPosition7::WaitImmobilizationForward,
                 ACESasibLeverPosition7::WaitImmobilizationBackwards};
    }
    else if(pos > ACESasibLeverPosition7::WaitImmobilizationBackwards &&
            pos < ACESasibLeverPosition7::WaitLiberationBackwards)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition7::WaitLiberationBackwards)
    {
        range = {ACESasibLeverPosition7::WaitLiberationBackwards,
                 ACESasibLeverPosition7::TurnedBackwards};
    }

    MechanicalInterface::LockRanges ranges;
    if(!empty)
    {
        ranges.append({int(range.first), int(range.second)});
    }

    mechanicalIface->setObjectLockConstraints(magnet(), ranges);
}
