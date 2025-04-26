/**
 * src/objects/lever/acei/aceileverobject.cpp
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

#include "aceileverobject.h"

#include "../../interfaces/leverinterface.h"

#include <QJsonObject>

static const EnumDesc acei_lever_posDesc =
{
    int(ACEILeverPosition::Left),
    int(ACEILeverPosition::Right),
    int(ACEILeverPosition::Vertical),
    "ACEILeverObject",
    {
        QT_TRANSLATE_NOOP("ACEILeverObject", "Left"),
        {},
        QT_TRANSLATE_NOOP("ACEILeverObject", "Vertical"),
        {},
        QT_TRANSLATE_NOOP("ACEILeverObject", "Right")
    }
};

static const LeverAngleDesc acei_lever_angleDesc =
{
    {-45, -90}, // Left
    {}, // Middle1
    {  0,   0}, // Vertical
    {}, // Middle2
    {+45, +90}  // Right
};

ACEILeverObject::ACEILeverObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject(m)
{
    leverInterface = new LeverInterface(acei_lever_posDesc,
                                        acei_lever_angleDesc,
                                        this);
    leverInterface->init();

    // Allow adding spring return
    leverInterface->setChangeSpringAllowed(true);
}

ACEILeverObject::~ACEILeverObject()
{
    delete leverInterface;
    leverInterface = nullptr;
}

QString ACEILeverObject::getType() const
{
    return Type;
}

bool ACEILeverObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase == LoadPhase::Creation)
    {
        setCanSealLeftPosition(obj.value("seal_left_pos").toBool(false));
    }

    return true;
}

void ACEILeverObject::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["seal_left_pos"] = canSealLeftPosition();
}

bool ACEILeverObject::canSealLeftPosition() const
{
    return mCanSealLeftPosition;
}

void ACEILeverObject::setCanSealLeftPosition(bool newCanSealLeftPosition)
{
    if(mCanSealLeftPosition == newCanSealLeftPosition)
        return;

    mCanSealLeftPosition = newCanSealLeftPosition;

    // Initially lock based on default
    setIsLeftPositionSealed(mCanSealLeftPosition);

    emit settingsChanged(this);
}

bool ACEILeverObject::isLeftPositionSealed() const
{
    return mIsLeftPositionSealed;
}

void ACEILeverObject::setIsLeftPositionSealed(bool newIsLeftPositionSealed)
{
    if(mIsLeftPositionSealed == newIsLeftPositionSealed)
        return;

    mIsLeftPositionSealed = newIsLeftPositionSealed;

    if(mIsLeftPositionSealed)
    {
        const int min = std::max(leverInterface->absoluteMin(),
                                 int(ACEILeverPosition::Vertical));
        leverInterface->setLockedRange(min, leverInterface->absoluteMax());
        leverInterface->checkPositionValidForLock();
    }
    else
    {
        leverInterface->setLockedRange(LeverAngleDesc::InvalidPosition,
                                       LeverAngleDesc::InvalidPosition);
    }

    emit stateChanged(this);
}
