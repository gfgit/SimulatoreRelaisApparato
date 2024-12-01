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
    {-90, -90}, // Left
    {}, // Middle1
    {  0,   0}, // Vertical
    {}, // Middle2
    {+90, +90}  // Right
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
