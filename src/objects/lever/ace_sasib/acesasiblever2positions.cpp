/**
 * src/objects/lever/ace_sasib/acesasiblever2positions.cpp
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

#include "acesasiblever2positions.h"

#include "../../interfaces/mechanicalinterface.h"
#include "../../interfaces/leverinterface.h"
#include "../../interfaces/sasibaceleverextrainterface.h"

#include "../../simple_activable/electromagnet.h"

static const EnumDesc ace_sasib_2_posDesc =
{
    int(ACESasibLeverPosition2::Reverse),
    int(ACESasibLeverPosition2::Normal),
    int(ACESasibLeverPosition2::Normal),
    "ACESasibLever2PosObject",
    {
        QT_TRANSLATE_NOOP("ACESasibLever2PosObject", "Reverse"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever2PosObject", "Check circuits R"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever2PosObject", "Wait control R"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever2PosObject", "Wait control N"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever2PosObject", "Check circuits N"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever2PosObject", "Normal")
    }
};

static const LeverAngleDesc ace_sasib_2_angleDesc =
{
    {-80, -135}, // Reverse
    {}, // Middle1
    {-50, -90}, // Check circuits R
    {}, // Middle2
    {-30, -45}, // Wait control R

    {}, // Middle3

    {+30, +45}, // Wait control N
    {}, // Middle4
    {+50, +90}, // Check circuits N
    {}, // Middle5
    {+80, +135}, // Normal
};

static QString ACESasibLever2PosObject_getCondName(AbstractSimulationObject *obj,
                                                   MechanicalCondition::Type t,
                                                   const MechanicalCondition::LockRange& r)
{
    if(!obj)
        return QString();

    switch (t)
    {
    case MechanicalCondition::Type::ExactPos:
    {
        if(r.first == int(ACESasibLeverPosition2::Normal))
            return obj->name();
        else if(r.first == int(ACESasibLeverPosition2::Reverse))
            return QLatin1String("<u>%1</u>").arg(obj->name());
        break;
    }
    default:
        break;
    }

    return QString();
}

ACESasibLever2PosObject::ACESasibLever2PosObject(AbstractSimulationObjectModel *m)
    : ACESasibLeverCommonObject(m, ace_sasib_2_posDesc, ace_sasib_2_angleDesc)
{
    // After all interfaces are constructed
    mechanicalIface->init();
    leverInterface->init();

    leverInterface->setChangeRangeAllowed(false);
    mechanicalIface->setAllowedConditionTypes({MechanicalCondition::Type::ExactPos});
    mechanicalIface->setCondNameFunc(&ACESasibLever2PosObject_getCondName);

    mechanicalIface->setLockablePositions(
                {
                    int(ACESasibLeverPosition2::Normal),
                    int(ACESasibLeverPosition2::Reverse)
                });

    mechanicalIface->addConditionSet(tr("Leave Normal"));
    mechanicalIface->setConditionSetRange(0,
                                          {int(ACESasibLeverPosition2::Normal),
                                           int(ACESasibLeverPosition2::Normal)});

    mechanicalIface->addConditionSet(tr("Leave Reverse"));
    mechanicalIface->setConditionSetRange(1,
                                          {int(ACESasibLeverPosition2::Reverse),
                                           int(ACESasibLeverPosition2::Reverse)});
}

QString ACESasibLever2PosObject::getType() const
{
    return Type;
}

void ACESasibLever2PosObject::addElectromagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition2 pos = ACESasibLeverPosition2(leverInterface->position());

    std::pair<ACESasibLeverPosition2, ACESasibLeverPosition2> range;
    bool empty = false;

    if(pos <= ACESasibLeverPosition2::CheckCircuitReverse)
    {
        range = {ACESasibLeverPosition2::Reverse,
                 ACESasibLeverPosition2::CheckCircuitReverse};
    }
    else if(pos > ACESasibLeverPosition2::CheckCircuitReverse &&
            pos < ACESasibLeverPosition2::WaitSwitchReverse)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition2::WaitSwitchReverse &&
            pos <= ACESasibLeverPosition2::WaitSwitchNormal)
    {
        range = {ACESasibLeverPosition2::WaitSwitchReverse,
                 ACESasibLeverPosition2::WaitSwitchNormal};
    }
    else if(pos > ACESasibLeverPosition2::WaitSwitchNormal &&
            pos < ACESasibLeverPosition2::CheckCircuitNormal)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition2::CheckCircuitNormal)
    {
        range = {ACESasibLeverPosition2::CheckCircuitNormal,
                 ACESasibLeverPosition2::Normal};
    }

    MechanicalInterface::LockRanges ranges;
    if(!empty)
    {
        ranges.append({int(range.first), int(range.second)});
    }

    mechanicalIface->setObjectLockConstraints(magnet(), ranges);
}

void ACESasibLever2PosObject::updateButtonsMagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition2 pos = ACESasibLeverPosition2(leverInterface->position());

    bool checkCircuit = (pos == ACESasibLeverPosition2::CheckCircuitNormal ||
                         pos == ACESasibLeverPosition2::CheckCircuitReverse);
    bool waitSwitch = (pos == ACESasibLeverPosition2::WaitSwitchNormal ||
                       pos == ACESasibLeverPosition2::WaitSwitchReverse);

    // Lock left button Tb if not check circuit
    sasibInterface->setButtonLocked(!checkCircuit,
                                    SasibACELeverExtraInterface::Button::Left);

    // Lock right button Tf if not wait for switch feedback
    sasibInterface->setButtonLocked(!waitSwitch,
                                    SasibACELeverExtraInterface::Button::Right);

    // Lock in down position the electromagnet if not in buttons positions
    if(mMagnet)
        mMagnet->setForcedOff(!checkCircuit && !waitSwitch);
}
