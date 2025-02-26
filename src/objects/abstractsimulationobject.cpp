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

#include "interfaces/abstractobjectinterface.h"

#include "../circuits/nodes/abstractcircuitnode.h"

#include <QTimerEvent>

#include <QJsonObject>

AbstractSimulationObject::AbstractSimulationObject(AbstractSimulationObjectModel *m)
    : QObject{m}
    , mModel(m)
{

}

AbstractSimulationObject::~AbstractSimulationObject()
{
    const auto ifaceListCopy = mInterfaces;
    for(AbstractObjectInterface *iface : ifaceListCopy)
        delete iface;
    Q_ASSERT(mInterfaces.isEmpty());

    Q_ASSERT(mTrackedObjects.isEmpty());

    emit destroyed(this);
}

AbstractObjectInterface *AbstractSimulationObject::getAbstractInterface(const QString &ifaceType) const
{
    for(AbstractObjectInterface *iface : std::as_const(mInterfaces))
    {
        if(iface->ifaceType() == ifaceType)
            return iface;
    }
    return nullptr;
}

bool AbstractSimulationObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(phase == LoadPhase::Creation)
    {
        if(obj.value("type") != getType())
            return false;

        setName(obj.value("name").toString());
        setDescription(obj.value("description").toString());
    }

    const QJsonObject ifaceListObj = obj.value("interfaces").toObject();
    for(AbstractObjectInterface *iface : std::as_const(mInterfaces))
    {
        const QJsonObject ifaceObj = ifaceListObj.value(iface->ifaceType()).toObject();
        if(!iface->loadFromJSON(ifaceObj, phase))
            return false;
    }

    return true;
}

void AbstractSimulationObject::saveToJSON(QJsonObject &obj) const
{
    obj["type"] = getType();
    obj["name"] = mName;
    obj["description"] = mDescription;

    QJsonObject ifaceListObj;
    for(AbstractObjectInterface *iface : std::as_const(mInterfaces))
    {
        QJsonObject ifaceObj;
        iface->saveToJSON(ifaceObj);

        ifaceListObj[iface->ifaceType()] = ifaceObj;
    }

    obj["interfaces"] = ifaceListObj;
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
    if(!isValid)
        return false;

    int nodesCount = getReferencingNodes(nullptr);
    QVector<AbstractCircuitNode *> nodesVec;
    nodesVec.reserve(nodesCount);

    getReferencingNodes(&nodesVec);

    for(AbstractCircuitNode *node : nodesVec)
    {
        // Prepare node geometry change
        emit node->shapeChanged(true);
    }

    // Do change name
    mName = trimmedName;

    for(AbstractCircuitNode *node : nodesVec)
    {
        // Trigger drawing update
        // TODO: find better way
        // NOTE: we want to avoid having to connecto to nameChanged()
        // in each node since there are many
        emit node->shapeChanged(false);
    }

    emit settingsChanged(this);
    emit nameChanged(mName);

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

int AbstractSimulationObject::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int nodesCount = 0;

    for(AbstractObjectInterface *iface : std::as_const(mInterfaces))
    {
        nodesCount += iface->getReferencingNodes(result);
    }

    return nodesCount;
}

void AbstractSimulationObject::onTrackedObjectDestroyed_slot(QObject *obj)
{
    AbstractSimulationObject *simObj = static_cast<AbstractSimulationObject *>(obj);

    onTrackedObjectDestroyed(simObj);

    Q_ASSERT(!mTrackedObjects.contains(simObj));
}

void AbstractSimulationObject::timerEvent(QTimerEvent *e)
{
    for(AbstractObjectInterface *iface : std::as_const(mInterfaces))
    {
        if(iface->timerEvent(e->timerId()))
            return;
    }

    QObject::timerEvent(e);
}

void AbstractSimulationObject::addInterface(AbstractObjectInterface *iface)
{
    mInterfaces.append(iface);
}

void AbstractSimulationObject::removeInterface(AbstractObjectInterface *iface)
{
    mInterfaces.removeOne(iface);
}

void AbstractSimulationObject::onInterfaceChanged(AbstractObjectInterface *iface, const QString &propName, const QVariant &value)
{
    emit interfacePropertyChanged(iface->ifaceType(),
                                  propName,
                                  value);
}

void AbstractSimulationObject::trackObject(AbstractSimulationObject *obj)
{
    Q_ASSERT(obj != this);

    auto it = mTrackedObjects.find(obj);
    if(it == mTrackedObjects.end())
    {
        connect(obj, &AbstractSimulationObject::destroyed,
                this, &AbstractSimulationObject::onTrackedObjectDestroyed_slot);
        it = mTrackedObjects.insert(obj, 0);
    }

    it.value()++;
}

void AbstractSimulationObject::untrackObject(AbstractSimulationObject *obj)
{
    auto it = mTrackedObjects.find(obj);
    Q_ASSERT(it != mTrackedObjects.end());

    it.value()--;
    if(it.value() == 0)
    {
        disconnect(obj, &AbstractSimulationObject::destroyed,
                   this, &AbstractSimulationObject::onTrackedObjectDestroyed_slot);
        mTrackedObjects.erase(it);
    }
}

void AbstractSimulationObject::onTrackedObjectDestroyed(AbstractSimulationObject *obj)
{
    for(AbstractObjectInterface *iface : std::as_const(mInterfaces))
    {
        iface->onTrackedObjectDestroyed(obj);
    }
}
