/**
 * src/objects/interfaces/abstractobjectinterface.cpp
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

#include "abstractobjectinterface.h"

#include "../abstractsimulationobject.h"

AbstractObjectInterface::AbstractObjectInterface(AbstractSimulationObject *obj)
    : mObject(obj)
{
    mObject->addInterface(this);
}

AbstractObjectInterface::~AbstractObjectInterface()
{
    mObject->removeInterface(this);
}

int AbstractObjectInterface::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    Q_UNUSED(result);
    return 0;
}

bool AbstractObjectInterface::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    Q_UNUSED(obj)
    Q_UNUSED(phase)
    return true;
}

void AbstractObjectInterface::saveToJSON(QJsonObject &obj) const
{
    Q_UNUSED(obj)
}

bool AbstractObjectInterface::timerEvent(const int timerId)
{
    Q_UNUSED(timerId)
    return false;
}

void AbstractObjectInterface::onTrackedObjectDestroyed(AbstractSimulationObject *obj)
{
    Q_UNUSED(obj)
}

void AbstractObjectInterface::emitChanged(const QString &propName, const QVariant &value)
{
    mObject->onInterfaceChanged(this, propName, value);
}

void AbstractObjectInterface::trackObject(AbstractSimulationObject *obj)
{
    mObject->trackObject(obj);
}

void AbstractObjectInterface::untrackObject(AbstractSimulationObject *obj)
{
    mObject->untrackObject(obj);
}
