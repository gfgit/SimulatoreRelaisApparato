/**
 * src/network/replicaobjectmanager.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#include "replicaobjectmanager.h"

#include "remotemanager.h"
#include "remotesession.h"
#include "remotesessionsmodel.h"
#include "replicasmodel.h"

#include "../views/modemanager.h"

#include "../objects/abstractsimulationobject.h"
#include "../objects/abstractsimulationobjectmodel.h"

#include <QCborMap>

#include <QJsonObject>
#include <QJsonArray>

ReplicaObjectManager::ReplicaObjectManager(RemoteManager *mgr)
    : QObject{mgr}
{
    mReplicasModel = new ReplicasModel(this);
}

ReplicaObjectManager::~ReplicaObjectManager()
{
    clear();

    delete mReplicasModel;
    mReplicasModel = nullptr;
}

RemoteManager *ReplicaObjectManager::remoteMgr() const
{
    return static_cast<RemoteManager *>(parent());
}

bool ReplicaObjectManager::addReplicaObject(AbstractSimulationObject *replicaObj)
{
    auto replicaIt = std::find_if(mReplicas.constBegin(),
                                  mReplicas.constEnd(),
                                  [replicaObj](const ReplicaObjectData& repData) -> bool
    {
        return repData.replicaObj == replicaObj;
    });

    if(replicaIt != mReplicas.constEnd())
        return false; // Already added

    connect(replicaObj, &AbstractSimulationObject::nameChanged,
            this, &ReplicaObjectManager::onReplicaNameChanged);
    connect(replicaObj, &AbstractSimulationObject::destroyed,
            this, &ReplicaObjectManager::onReplicaDestroyed);

    mReplicasModel->addItemAt(mReplicas.size(), true);
    mReplicas.append({replicaObj, nullptr, QString()});
    mReplicasModel->addItemAt(0, false);
    return true;
}

bool ReplicaObjectManager::removeReplicaObject(AbstractSimulationObject *replicaObj)
{
    auto replicaIt = std::find_if(mReplicas.begin(),
                                  mReplicas.end(),
                                  [replicaObj](const ReplicaObjectData& repData) -> bool
    {
        return repData.replicaObj == replicaObj;
    });

    if(replicaIt == mReplicas.end())
        return false;

    if(replicaIt->remoteSession)
    {
        const QString oldName = replicaIt->customName.isEmpty() ? replicaObj->name() : replicaIt->customName;
        replicaIt->remoteSession->removeReplica(replicaObj, oldName);
    }

    disconnect(replicaObj, &AbstractSimulationObject::nameChanged,
               this, &ReplicaObjectManager::onReplicaNameChanged);
    disconnect(replicaObj, &AbstractSimulationObject::destroyed,
               this, &ReplicaObjectManager::onReplicaDestroyed);

    mReplicasModel->removeItemAt(std::distance(mReplicas.begin(), replicaIt), true);
    mReplicas.erase(replicaIt);
    mReplicasModel->removeItemAt(0, false);

    return true;
}

bool ReplicaObjectManager::setReplicaObjectSession(AbstractSimulationObject *replicaObj,
                                                   RemoteSession *remoteSession, const QString &customName)
{
    auto replicaIt = std::find_if(mReplicas.begin(),
                                  mReplicas.end(),
                                  [replicaObj](const ReplicaObjectData& repData) -> bool
    {
        return repData.replicaObj == replicaObj;
    });

    if(replicaIt == mReplicas.end())
        return false;

    if(replicaIt->remoteSession == remoteSession && replicaIt->customName == customName)
        return true;

    if(replicaIt->remoteSession)
    {
        const QString oldName = replicaIt->customName.isEmpty() ? replicaObj->name() : replicaIt->customName;
        replicaIt->remoteSession->removeReplica(replicaObj, oldName);
    }

    replicaIt->remoteSession = remoteSession;
    replicaIt->customName = customName;

    if(replicaIt->remoteSession)
    {
        const QString newName = replicaIt->customName.isEmpty() ? replicaObj->name() : replicaIt->customName;
        replicaIt->remoteSession->addReplica(replicaObj, newName);
    }

    const QModelIndex idx = mReplicasModel->index(std::distance(mReplicas.begin(), replicaIt),
                                                  ReplicasModel::SessionDeviceCol);
    emit mReplicasModel->dataChanged(idx, idx);

    return true;
}

bool ReplicaObjectManager::loadFromJSON(const QJsonObject &obj)
{
    const QJsonArray replicas = obj["replicas"].toArray();

    ModeManager *modeMgr = remoteMgr()->modeMgr();

    struct RepData
    {
        AbstractSimulationObject *replicaObj = nullptr;
        QString remoteSessionName;
        QString customName;
    };

    QVector<RepData> sortedReplicas;

    for(const QJsonValue& v : replicas)
    {
        QJsonObject replica = v.toObject();
        RepData repData;

        const QString objName = replica.value("object").toString();
        const QString objType = replica.value("object_type").toString();
        if(objName.isEmpty() || objType.isEmpty())
            continue;

        auto model = modeMgr->modelForType(objType);
        if(!model)
            continue;

        repData.replicaObj = model->getObjectByName(objName);
        if(!repData.replicaObj)
            continue;

        repData.customName = replica.value("custom_source_name").toString();
        repData.remoteSessionName = replica.value("remote_session").toString();

        sortedReplicas.append(repData);
    }

    std::sort(sortedReplicas.begin(),
              sortedReplicas.end(),
              [](const RepData& lhs, const RepData& rhs) -> bool
    {
        const QString nameA = lhs.replicaObj->name();
        const QString nameB = rhs.replicaObj->name();

        if(nameA == nameB)
        {
            // Same name, order by type
            return lhs.replicaObj->getType() < rhs.replicaObj->getType();
        }

        // Order by name
        return nameA < nameB;
    });

    for(const RepData& val : std::as_const(sortedReplicas))
    {
        if(!addReplicaObject(val.replicaObj))
            continue;

        RemoteSession *remoteSession = nullptr;
        if(!val.remoteSessionName.isEmpty())
        {
            remoteSession = remoteMgr()->addRemoteSession(val.remoteSessionName);
        }

        if(remoteSession)
            setReplicaObjectSession(val.replicaObj,
                                    remoteSession,
                                    val.customName);
    }

    return true;
}

void ReplicaObjectManager::saveToJSON(QJsonObject &obj)
{
    auto sortedReplicas = mReplicas;
    std::sort(sortedReplicas.begin(),
              sortedReplicas.end(),
              [](const ReplicaObjectData& lhs, const ReplicaObjectData& rhs) -> bool
    {
        const QString nameA = lhs.replicaObj->name();
        const QString nameB = rhs.replicaObj->name();

        if(nameA == nameB)
        {
            // Same name, order by type
            return lhs.replicaObj->getType() < rhs.replicaObj->getType();
        }

        // Order by name
        return nameA < nameB;
    });

    QJsonArray replicas;

    for(const ReplicaObjectData& repData : std::as_const(sortedReplicas))
    {
        QJsonObject replica;
        replica["object"] = repData.replicaObj->name();
        replica["object_type"] = repData.replicaObj->getType();
        replica["remote_session"] = repData.remoteSession ? repData.remoteSession->getSessionName() : QString();
        replica["custom_source_name"] = repData.customName;
        replicas.append(replica);
    }

    obj["replicas"] = replicas;
}

void ReplicaObjectManager::clear()
{
    const auto replicas = mReplicas;
    for(const ReplicaObjectData& repData : replicas)
        removeReplicaObject(repData.replicaObj);
}

void ReplicaObjectManager::onSourceObjStateChanged(AbstractSimulationObject *obj)
{
    auto it = mSourceObjects.constFind(obj);
    if(it == mSourceObjects.constEnd())
        return;

    AbstractSimulationObject *sourceObj = it.key();
    QCborMap objState;
    sourceObj->getReplicaState(objState);

    for(const RemoteSessionData& sessionData : it.value().sessions)
    {
        sessionData.remoteSession->sendSourceObjectState(sessionData.replicaId,
                                                         objState);
    }
}

void ReplicaObjectManager::onReplicaDestroyed(QObject *obj)
{
    removeReplicaObject(static_cast<AbstractSimulationObject *>(obj));
}

void ReplicaObjectManager::onReplicaNameChanged(AbstractSimulationObject *replicaObj,
                                                const QString &newName, const QString &oldName)
{
    auto replicaIt = std::find_if(mReplicas.begin(),
                                  mReplicas.end(),
                                  [replicaObj](const ReplicaObjectData& repData) -> bool
    {
        return repData.replicaObj == replicaObj;
    });

    if(replicaIt == mReplicas.end())
        return;

    if(!replicaIt->remoteSession || !replicaIt->customName.isEmpty())
        return;

    if(replicaIt->remoteSession)
    {
        replicaIt->remoteSession->removeReplica(replicaObj, oldName);
    }

    if(replicaIt->remoteSession)
    {
        replicaIt->remoteSession->addReplica(replicaObj, newName);
    }

    const QModelIndex idx = mReplicasModel->index(std::distance(mReplicas.begin(), replicaIt),
                                                  ReplicasModel::SessionDeviceCol);
    emit mReplicasModel->dataChanged(idx, idx);
}

void ReplicaObjectManager::addSourceObject(AbstractSimulationObject *obj,
                                           RemoteSession *remoteSession,
                                           quint64 replicaId)
{
    auto it = mSourceObjects.find(obj);
    if(it == mSourceObjects.end())
    {
        it = mSourceObjects.insert(obj, {});
        connect(obj, &AbstractSimulationObject::stateChanged,
                this, &ReplicaObjectManager::onSourceObjStateChanged);
    }

    it.value().sessions.append({remoteSession, replicaId});

    QCborMap objState;
    obj->getReplicaState(objState);
    remoteSession->sendSourceObjectState(replicaId, objState);
}

void ReplicaObjectManager::removeSourceObjects(RemoteSession *remoteSession)
{
    auto objIt = mSourceObjects.begin();
    while(objIt != mSourceObjects.end())
    {
        SourceObjectData& objData = objIt.value();
        auto sessionIt = std::find_if(objData.sessions.begin(),
                                      objData.sessions.end(),
                                      [remoteSession](const RemoteSessionData& remoteData) -> bool
        {
            return remoteData.remoteSession == remoteSession;
        });

        if(sessionIt != objData.sessions.end())
            objData.sessions.erase(sessionIt);

        if(objData.sessions.isEmpty())
        {
            disconnect(objIt.key(), &AbstractSimulationObject::stateChanged,
                       this, &ReplicaObjectManager::onSourceObjStateChanged);
            objIt = mSourceObjects.erase(objIt);
        }
        else
        {
            objIt++;
        }
    }
}

ReplicasModel *ReplicaObjectManager::replicasModel() const
{
    return mReplicasModel;
}


