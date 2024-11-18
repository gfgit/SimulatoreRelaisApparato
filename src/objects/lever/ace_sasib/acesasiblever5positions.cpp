/**
 * src/objects/lever/ace_sasib/acesasiblever5positions.cpp
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

#include "acesasiblever5positions.h"

#include "../../interfaces/mechanicalinterface.h"
#include "../../interfaces/leverinterface.h"

#include "../../simple_activable/electromagnet.h"

static const EnumDesc ace_sasib_5_posDesc =
{
    int(ACESasibLeverPosition5::Reverse),
    int(ACESasibLeverPosition5::Normal),
    int(ACESasibLeverPosition5::Normal),
    "ACESasibLever5PosObject",
    {
        QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Reverse"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Check circuits R"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Wait control R"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Wait control N"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Check circuits N"),
        {},
        QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Normal")
    }
};

static const LeverAngleDesc ace_sasib_5_angleDesc =
{
    {-70, -135}, // Reverse
    {}, // Middle1
    {-50, -90}, // Check circuits R
    {}, // Middle2
    {-30, -45}, // Wait control R

    {}, // Middle3

    {+30, +45}, // Wait control N
    {}, // Middle4
    {+50, +90}, // Check circuits N
    {}, // Middle5
    {+70, +135}, // Normal
};

ACESasibLever5PosObject::ACESasibLever5PosObject(AbstractSimulationObjectModel *m)
    : ACESasibLeverCommonObject(m, ace_sasib_5_posDesc, ace_sasib_5_angleDesc)
{
    leverInterface->setChangeRangeAllowed(false);
}

QString ACESasibLever5PosObject::getType() const
{
    return Type;
}

void ACESasibLever5PosObject::addElectromagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition5 pos = ACESasibLeverPosition5(leverInterface->position());

    std::pair<ACESasibLeverPosition5, ACESasibLeverPosition5> range;
    bool empty = false;

    if(pos <= ACESasibLeverPosition5::CheckCircuitReverse)
    {
        range = {ACESasibLeverPosition5::Reverse,
                 ACESasibLeverPosition5::CheckCircuitReverse};
    }
    else if(pos > ACESasibLeverPosition5::CheckCircuitReverse &&
            pos < ACESasibLeverPosition5::WaitSwitchReverse)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition5::WaitSwitchReverse &&
            pos <= ACESasibLeverPosition5::WaitSwitchNormal)
    {
        range = {ACESasibLeverPosition5::WaitSwitchReverse,
                 ACESasibLeverPosition5::WaitSwitchNormal};
    }
    else if(pos > ACESasibLeverPosition5::WaitSwitchNormal &&
            pos < ACESasibLeverPosition5::CheckCircuitNormal)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition5::CheckCircuitNormal)
    {
        range = {ACESasibLeverPosition5::CheckCircuitNormal,
                 ACESasibLeverPosition5::Normal};
    }

    MechanicalInterface::LockRanges ranges;
    if(!empty)
    {
        ranges.append({int(range.first), int(range.second)});
    }

    mechanicalIface->setObjectLockConstraints(magnet(), ranges);
}
