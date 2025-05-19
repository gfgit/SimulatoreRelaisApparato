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

#include "../objects/abstractsimulationobject.h"

#include <QCborMap>

ReplicaObjectManager::ReplicaObjectManager(RemoteManager *mgr)
    : QObject{mgr}
{

}

RemoteManager *ReplicaObjectManager::remoteMgr() const
{
    return static_cast<RemoteManager *>(parent());
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
        sessionData.remoteSession->sendSourceObjectState(sessionData.objectId,
                                                         objState);
    }
}

void ReplicaObjectManager::addSourceObject(AbstractSimulationObject *obj,
                                           RemoteSession *remoteSession,
                                           quint64 objectId)
{
    auto it = mSourceObjects.find(obj);
    if(it == mSourceObjects.end())
    {
        it = mSourceObjects.insert(obj, {});
        connect(obj, &AbstractSimulationObject::stateChanged,
                this, &ReplicaObjectManager::onSourceObjStateChanged);
    }

    it.value().sessions.append({remoteSession, objectId});

    QCborMap objState;
    obj->getReplicaState(objState);
    remoteSession->sendSourceObjectState(objectId, objState);
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


