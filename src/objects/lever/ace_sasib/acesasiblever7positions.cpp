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

#include "../../simple_activable/electromagnet.h"

static QString ACESasibLever7PosObject_translate(const char *nameId)
{
    return ACESasibLever7PosObject::tr(nameId);
}

static const LeverPositionDesc::Item ace_sasib_7_LeverItems[] =
{
    {-60, -135, QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Forward")},
    {}, // Middle1
    {-45, -90, QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Liberation F")},
    {}, // Middle2
    {-30, -45, QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Immobilization F")},
    {}, // Middle3
    {  0,   0, QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Normal")},
    {}, // Middle4
    {+30, +45, QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Immobilization B")},
    {}, // Middle5
    {+45, +90, QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Wait Liberation B")},
    {}, // Middle6
    {+60, +135, QT_TRANSLATE_NOOP("ACESasibLever7PosObject", "Backwards")}
};

static const LeverPositionDesc aceSasib7LeverDesc(ace_sasib_7_LeverItems,
                                                  int(ACESasibLeverPosition7::Normal),
                                                  &ACESasibLever7PosObject_translate);


ACESasibLever7PosObject::ACESasibLever7PosObject(AbstractSimulationObjectModel *m)
    : ACESasibLeverCommonObject(m, aceSasib7LeverDesc)
{

}

QString ACESasibLever7PosObject::getType() const
{
    return Type;
}

void ACESasibLever7PosObject::addElectromagnetLock()
{
    // Lock depends on current position
    ACESasibLeverPosition7 pos = ACESasibLeverPosition7(position());

    std::pair<ACESasibLeverPosition7, ACESasibLeverPosition7> range;
    bool empty = false;

    if(pos <= ACESasibLeverPosition7::WaitLiberationForward)
    {
        range = {ACESasibLeverPosition7::TurnedForward,
                 ACESasibLeverPosition7::WaitLiberationForward};
    }
    else if(pos < ACESasibLeverPosition7::WaitImmobilizationForward)
    {
        empty = true; // Magnet is sliding on top of lever
    }
    else if(pos >= ACESasibLeverPosition7::WaitImmobilizationForward &&
            pos <= ACESasibLeverPosition7::WaitImmobilizationBackwards)
    {
        range = {ACESasibLeverPosition7::WaitImmobilizationForward,
                 ACESasibLeverPosition7::WaitImmobilizationBackwards};
    }
    else if(pos >= ACESasibLeverPosition7::WaitLiberationBackwards)
    {
        range = {ACESasibLeverPosition7::WaitLiberationBackwards,
                 ACESasibLeverPosition7::TurnedBackwards};
    }
    else if(pos > ACESasibLeverPosition7::WaitImmobilizationBackwards)
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
