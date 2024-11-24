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

ACESasibLever7PosObject::ACESasibLever7PosObject(AbstractSimulationObjectModel *m)
    : ACESasibLeverCommonObject(m, ace_sasib_7_posDesc, ace_sasib_7_angleDesc)
{

}

QString ACESasibLever7PosObject::getType() const
{
    return Type;
}

bool ACESasibLever7PosObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    ACESasibLeverCommonObject::loadFromJSON(obj, phase);

    if(phase == LoadPhase::AllCreated)
    {
        // Conditions to go backwards
        MechanicalConditionSet set1;
        set1.allowedRangeWhenLocked = {int(ACESasibLeverPosition7::TurnedForward),
                                       int(ACESasibLeverPosition7::Normal)};

        // Conditions to go forward
        MechanicalConditionSet set2;
        set2.allowedRangeWhenLocked = {int(ACESasibLeverPosition7::Normal),
                                       int(ACESasibLeverPosition7::TurnedBackwards)};

        MechanicalCondition c;
        c.type = MechanicalCondition::Type::ExactPos;

        if(name() == "S32")
        {
            MechanicalCondition and_;
            and_.type = MechanicalCondition::Type::And;

            // Lock I24
            auto other = model()->getObjectByName("I24");
            if(other)
            {
                auto otherIface = other->getInterface<MechanicalInterface>();
                //c.type = MechanicalCondition::Type::ExactPos;
                c.type = MechanicalCondition::Type::RangePos;
                c.requiredPositions.first = int(ACESasibLeverPosition7::TurnedBackwards);
                c.requiredPositions.first = int(ACESasibLeverPosition7::WaitLiberationBackwards);
                c.otherIface = otherIface;
                and_.subConditions.append(c);
            }

            auto other2 = model()->getObjectByName("I25");
            if(other2)
            {
                MechanicalCondition c2;
                c2.type = MechanicalCondition::Type::NotPos;
                c2.requiredPositions.first = int(ACESasibLeverPosition7::TurnedBackwards);
                c2.otherIface = other2->getInterface<MechanicalInterface>();
                and_.subConditions.append(c2);
            }

            set1.rootCondition = and_;

            c.requiredPositions.first = int(ACESasibLeverPosition7::TurnedForward);
            set2.rootCondition = c;

            mechanicalIface->addConditionSet(set1);
            //mechanicalIface->addConditionSet(set2);

        }
        else if(name() == "I24")
        {
            auto other = model()->modeMgr()->modelForType(ACESasibLever5PosObject::Type)->getObjectByName("D13");

            if(other)
            {
                auto otherIface = other->getInterface<MechanicalInterface>();
                c.requiredPositions.first = int(ACESasibLeverPosition5::Normal);
                c.otherIface = otherIface;

                set1.rootCondition = c;

                c.requiredPositions.first = int(ACESasibLeverPosition5::Reverse);
                set2.rootCondition = c;

                mechanicalIface->addConditionSet(set1);
                mechanicalIface->addConditionSet(set2);
            }
        }
    }

    return true;
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
