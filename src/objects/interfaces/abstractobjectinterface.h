/**
 * src/objects/interfaces/abstractobjectinterface.h
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

#ifndef ABSTRACTOBJECTINTERFACE_H
#define ABSTRACTOBJECTINTERFACE_H

#include <QString>

#include "../../enums/loadphase.h"

class QVariant;

class AbstractSimulationObject;
class AbstractCircuitNode;

class QJsonObject;

class AbstractObjectInterface
{
public:
    AbstractObjectInterface(AbstractSimulationObject *obj);
    virtual ~AbstractObjectInterface();

    virtual QString ifaceType() = 0;

    virtual QVector<AbstractCircuitNode*> nodes() const;

    virtual bool loadFromJSON(const QJsonObject &obj, LoadPhase phase);
    virtual void saveToJSON(QJsonObject &obj) const;

    inline AbstractSimulationObject *object() const
    {
        return mObject;
    }

protected:
    friend class AbstractSimulationObject;
    virtual bool timerEvent(const int timerId);

    virtual void onTrackedObjectDestroyed(AbstractSimulationObject *obj);

    void emitChanged(const QString& propName, const QVariant& value);

    void trackObject(AbstractSimulationObject *obj);
    void untrackObject(AbstractSimulationObject *obj);

protected:
    AbstractSimulationObject * const mObject;
};

#endif // ABSTRACTOBJECTINTERFACE_H
