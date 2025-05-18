/**
 * src/objects/lever/ace_sasib/acesasiblever3positions.cpp
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

#include "acesasiblever3positions.h"

#include "../../interfaces/mechanicalinterface.h"
#include "../../interfaces/leverinterface.h"
#include "../../interfaces/sasibaceleverextrainterface.h"

#include "../../simple_activable/electromagnet.h"



#include "../../abstractsimulationobjectmodel.h"
#include "acesasiblever2positions.h"

#include "../../../views/modemanager.h"

static const EnumDesc ace_sasib_3_posDesc =
{
    int(ACESasibLeverPosition3::TurnedForward),
    int(ACESasibLeverPosition3::TurnedBackwards),
    int(ACESasibLeverPosition3::Normal),
    "ACESasibLever3PosObject",
    {
        QT_TRANSLATE_NOOP("ACESasibLever3PosObject", "Forward"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever3PosObject", "Wait Liberation F"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever3PosObject", "Wait Immobilization F"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever3PosObject", "Normal"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever3PosObject", "Wait Immobilization B"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever3PosObject", "Wait Liberation B"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever3PosObject", "Backwards")
    }
};

static const LeverAngleDesc ace_sasib_3_angleDesc =
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

inline QLatin1String getPosSuffix(ACESasibLeverPosition3 pos)
{
    switch (pos)
    {
    case ACESasibLeverPosition3::TurnedForward:
    {
        return QLatin1String("a");
    }
    case ACESasibLeverPosition3::WaitLiberationForward:
    {
        return QLatin1String("la");
    }
    case ACESasibLeverPosition3::WaitLiberationBackwards:
    {
        return QLatin1String("li");
    }
    case ACESasibLeverPosition3::TurnedBackwards:
    {
        return QLatin1String("i");
    }
    default:
        break;
    }

    return QLatin1String();
}

static QString ACESasibLever3PosObject_getCondName(AbstractSimulationObject *obj,
                                                   MechanicalCondition::Type t,
                                                   const MechanicalCondition::LockRange& r)
{
    if(!obj)
        return QString();

    static const QLatin1String suffixFmt("<u>%1%2</u>");
    ACESasibLeverPosition3 pos = ACESasibLeverPosition3(r.first);

    switch (t)
    {
    case MechanicalCondition::Type::ExactPos:
    {
        if(pos == ACESasibLeverPosition3::Normal)
            return obj->name();

        QLatin1String suffix = getPosSuffix(pos);
        if(suffix.isEmpty())
            return QString();

        return suffixFmt.arg(obj->name(), suffix);
    }
    case MechanicalCondition::Type::RangePos:
    {
        ACESasibLeverPosition3 pos2 = ACESasibLeverPosition3(r.second);

        QString result;

        if(pos == ACESasibLeverPosition3::Normal)
            result = obj->name();
        else
        {
            QLatin1String suffix = getPosSuffix(pos);
            if(suffix.isEmpty())
                return QString();

            result = suffixFmt.arg(obj->name(), suffix);
        }

        result += QLatin1String("-");

        if(pos2 == ACESasibLeverPosition3::Normal)
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
        if(pos == ACESasibLeverPosition3::Normal)
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


ACESasibLever3PosObject::ACESasibLever3PosObject(AbstractSimulationObjectModel *m)
    : ACESasibLeverCommonObject(m, ace_sasib_3_posDesc, ace_sasib_3_angleDesc)
{
    // After all interfaces are constructed
    mechanicalIface->init();
    leverInterface->init();

    mechanicalIface->setAllowedConditionTypes(
                {
                    MechanicalCondition::Type::ExactPos,
                    MechanicalCondition::Type::RangePos,
                    MechanicalCondition::Type::NotPos
                });
    mechanicalIface->setCondNameFunc(&ACESasibLever3PosObject_getCondName);

    mechanicalIface->setLockablePositions(
                {
                    int(ACESasibLeverPosition3::TurnedForward),
                    int(ACESasibLeverPosition3::WaitLiberationForward),
                    int(ACESasibLeverPosition3::Normal),
                    int(ACESasibLeverPosition3::WaitLiberationBackwards),
                    int(ACESasibLeverPosition3::TurnedBackwards)
                });

    mechanicalIface->addConditionSet(tr("Go Backwards"));
    mechanicalIface->setConditionSetRange(0,
                                          {int(ACESasibLeverPosition3::TurnedForward),
                                           int(ACESasibLeverPosition3::Normal)});

    mechanicalIface->addConditionSet(tr("Go Forward"));
    mechanicalIface->setConditionSetRange(1,
                                          {int(ACESasibLeverPosition3::Normal),
                                           int(ACESasibLeverPosition3::TurnedBackwards)});
}

QString ACESasibLever3PosObject::getType() const
{
    return Type;
}

void ACESasibLever3PosObject::addElectromagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition3 pos = ACESasibLeverPosition3(leverInterface->position());

    std::pair<ACESasibLeverPosition3, ACESasibLeverPosition3> range;
    bool empty = false;

    if(pos <= ACESasibLeverPosition3::WaitLiberationForward)
    {
        range = {ACESasibLeverPosition3::TurnedForward,
                 ACESasibLeverPosition3::WaitLiberationForward};
    }
    else if(pos > ACESasibLeverPosition3::WaitLiberationForward &&
            pos < ACESasibLeverPosition3::WaitImmobilizationForward)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition3::WaitImmobilizationForward &&
            pos <= ACESasibLeverPosition3::WaitImmobilizationBackwards)
    {
        // There is Normal position inside this range
        range = {ACESasibLeverPosition3::WaitImmobilizationForward,
                 ACESasibLeverPosition3::WaitImmobilizationBackwards};
    }
    else if(pos > ACESasibLeverPosition3::WaitImmobilizationBackwards &&
            pos < ACESasibLeverPosition3::WaitLiberationBackwards)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition3::WaitLiberationBackwards)
    {
        range = {ACESasibLeverPosition3::WaitLiberationBackwards,
                 ACESasibLeverPosition3::TurnedBackwards};
    }

    MechanicalInterface::LockRanges ranges;
    if(!empty)
    {
        ranges.append({int(range.first), int(range.second)});
    }

    mechanicalIface->setObjectLockConstraints(magnet(), ranges);
}

void ACESasibLever3PosObject::updateButtonsMagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition3 pos = ACESasibLeverPosition3(leverInterface->position());

    bool waitImmobilization = (pos == ACESasibLeverPosition3::WaitImmobilizationBackwards ||
                               pos == ACESasibLeverPosition3::WaitImmobilizationForward);
    bool waitLiberation = (pos == ACESasibLeverPosition3::WaitLiberationBackwards ||
                           pos == ACESasibLeverPosition3::WaitLiberationForward);

    // Lock left button Tb if not waiting immobilization
    sasibInterface->setButtonLocked(!waitImmobilization,
                                    SasibACELeverExtraInterface::Button::Left);

    // Lock right button Tl if not waiting liberation
    sasibInterface->setButtonLocked(!waitLiberation,
                                    SasibACELeverExtraInterface::Button::Right);

    // Lock in down position the electromagnet if not in buttons positions
    if(mMagnet)
        mMagnet->setForcedOff(!waitImmobilization && !waitLiberation);
}
