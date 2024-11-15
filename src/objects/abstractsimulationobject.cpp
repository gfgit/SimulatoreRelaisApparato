/**
 * src/objects/abstractsimulationobject.cpp
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

#include "abstractsimulationobject.h"
#include "abstractsimulationobjectmodel.h"

#include "../circuits/nodes/abstractcircuitnode.h"

#include <QJsonObject>

AbstractSimulationObject::AbstractSimulationObject(AbstractSimulationObjectModel *m)
    : QObject{m}
    , mModel(m)
{

}

QObject *AbstractSimulationObject::getInterface(const QString &ifaceName)
{
    Q_UNUSED(ifaceName)
    return nullptr;
}

bool AbstractSimulationObject::loadFromJSON(const QJsonObject &obj)
{
    if(obj.value("type") != getType())
        return false;

    setName(obj.value("name").toString());
    setDescription(obj.value("description").toString());
    return true;
}

void AbstractSimulationObject::saveToJSON(QJsonObject &obj) const
{
    obj["type"] = getType();
    obj["name"] = mName;
    obj["description"] = mDescription;
}

QString AbstractSimulationObject::name() const
{
    return mName;
}

bool AbstractSimulationObject::setName(const QString &newName)
{
    const QString trimmedName = newName.trimmed();

    if(mName == trimmedName)
        return true;

    bool isValid = model()->isNameAvailable(trimmedName);

    if(isValid)
    {
        // Do change name
        mName = trimmedName;

        emit settingsChanged(this);
    }

    emit nameChanged(mName);

    for(AbstractCircuitNode *node : nodes())
    {
        // Trigger drawind update
        // TODO: find better way
        // NOTE: we want to avoid having to connecto to nameChanged()
        // in each node since there are many
        emit node->circuitsChanged();
    }

    return isValid;
}

QString AbstractSimulationObject::description() const
{
    return mDescription;
}

void AbstractSimulationObject::setDescription(const QString &newDescription)
{
    QString value = newDescription.trimmed();
    if (mDescription == value)
        return;

    mDescription = value;
    emit descriptionChanged(mDescription);
}

QVector<AbstractCircuitNode *> AbstractSimulationObject::nodes() const
{
    return {};
}
