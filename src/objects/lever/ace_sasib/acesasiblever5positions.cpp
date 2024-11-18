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

#include "../../simple_activable/electromagnet.h"

static QString ACESasibLever5PosObject_translate(const char *nameId)
{
    return ACESasibLever5PosObject::tr(nameId);
}

static const LeverPositionDesc::Item ace_sasib_5_LeverItems[] =
{
    {-60, -135, QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Normal")},
    {}, // Middle1
    {-45, -90, QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Check circuits R")},
    {}, // Middle2
    {-30, -45, QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Wait control N")},

    {}, // Middle3

    {+30, +45, QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Wait control R")},
    {}, // Middle4
    {+45, +90, QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Check circuits N")},
    {}, // Middle5
    {+60, +135, QT_TRANSLATE_NOOP("ACESasibLever5PosObject", "Reverse")}
};

static const LeverPositionDesc aceSasib5LeverDesc(ace_sasib_5_LeverItems,
                                                  int(ACESasibLeverPosition5::Normal),
                                                  &ACESasibLever5PosObject_translate);


ACESasibLever5PosObject::ACESasibLever5PosObject(AbstractSimulationObjectModel *m)
    : ACESasibLeverCommonObject(m, aceSasib5LeverDesc)
{

}

QString ACESasibLever5PosObject::getType() const
{
    return Type;
}

void ACESasibLever5PosObject::addElectromagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition5 pos = ACESasibLeverPosition5(position());

    std::pair<ACESasibLeverPosition5, ACESasibLeverPosition5> range;
    bool empty = false;

    if(pos <= ACESasibLeverPosition5::CheckCircuitForward)
    {
        range = {ACESasibLeverPosition5::Normal,
                 ACESasibLeverPosition5::CheckCircuitForward};
    }
    else if(pos < ACESasibLeverPosition5::WaitSwitchNormal)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition5::WaitSwitchNormal &&
            pos <= ACESasibLeverPosition5::WaitSwitchReverse)
    {
        range = {ACESasibLeverPosition5::WaitSwitchNormal,
                 ACESasibLeverPosition5::WaitSwitchReverse};
    }
    else if(pos >= ACESasibLeverPosition5::CheckCircuitReverse)
    {
        range = {ACESasibLeverPosition5::CheckCircuitReverse,
                 ACESasibLeverPosition5::Reverse};
    }
    else if(pos > ACESasibLeverPosition5::WaitSwitchReverse)
    {
        empty = true; // Magnet is sliding on top of lever
    }

    MechanicalInterface::LockRanges ranges;
    if(!empty)
    {
        ranges.append({int(range.first), int(range.second)});
    }

    mechanicalIface->setObjectLockConstraints(magnet(), ranges);
}
