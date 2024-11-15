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

#include "../../../circuits/nodes/levercontactnode.h"

#include <QTimerEvent>

#include <QJsonObject>

static QString ACEILeverPosition_translate(const char *nameId)
{
    return ACEILeverObject::tr(nameId);
}

static const LeverPositionDesc::Item aceiLeverItems[] =
{
    {-90, -90, QT_TRANSLATE_NOOP("ACEILeverObject", "Left")},
    {}, // Middle1
    {  0,   0, QT_TRANSLATE_NOOP("ACEILeverObject", "Normal")},
    {}, // Middle2
    {+90, +90, QT_TRANSLATE_NOOP("ACEILeverObject", "Right")}
};

static const LeverPositionDesc aceiLeverDesc(aceiLeverItems,
                                             int(ACEILeverPosition::Normal),
                                             &ACEILeverPosition_translate);

ACEILeverObject::ACEILeverObject(AbstractSimulationObjectModel *m)
    : GenericLeverObject(m, aceiLeverDesc)
{

}

QString ACEILeverObject::getType() const
{
    return Type;
}
