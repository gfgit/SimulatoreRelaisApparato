/**
 * src/network/remotesession.cpp
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

#include "remotesession.h"
#include "remotesessionsmodel.h"

#include "remotemanager.h"
#include "peerconnection.h"
#include "replicaobjectmanager.h"

#include "../views/modemanager.h"

#include "../objects/circuit_bridge/remotecircuitbridge.h"
#include "../objects/circuit_bridge/remotecircuitbridgesmodel.h"

#include <QCborMap>
#include <QCborArray>

RemoteSession::RemoteSession(const QString &sessionName, RemoteManager *remoteMgr)
    : QObject{remoteMgr}
    , mSessionName(sessionName)
{

}

RemoteSession::~RemoteSession()
{
    Q_ASSERT(!mPeerConn);

    const auto bridges = mBridges;
    for(RemoteCircuitBridge *bridge : bridges)
    {
        bridge->setRemoteSession(nullptr);
    }
    Q_ASSERT(mBridges.isEmpty());

    ReplicaObjectManager *replicaMgr = remoteMgr()->replicaMgr();
    const auto replicas = mReplicas;
    for(const ReplicaData& repData : replicas)
    {
        for(AbstractSimulationObject *replica : repData.objects)
        {
            replicaMgr->removeReplicaObject(replica);
        }
    }
    Q_ASSERT(mReplicas.isEmpty());
}

RemoteManager *RemoteSession::remoteMgr() const
{
    return static_cast<RemoteManager *>(parent());
}

bool RemoteSession::setSessionName(const QString &newName)
{
    const QString nameTrimmed = newName.trimmed();
    if(nameTrimmed.isEmpty() || nameTrimmed == mSessionName)
        return false;

    if(!remoteMgr()->renameRemoteSession(mSessionName, nameTrimmed))
        return false;

    mSessionName = nameTrimmed;

    ModeManager *modeMgr = remoteMgr()->modeMgr();
    modeMgr->setFileEdited();

    RemoteCircuitBridgesModel *bridgesModel = static_cast<RemoteCircuitBridgesModel *>(
                modeMgr->modelForType(RemoteCircuitBridge::Type));
    bridgesModel->updateRemoteSessions();

    return true;
}

void RemoteSession::addRemoteBridge(RemoteCircuitBridge *bridge)
{
    mBridges.append(bridge);
}

void RemoteSession::removeRemoteBridge(RemoteCircuitBridge *bridge)
{
    mBridges.removeOne(bridge);
}

QHostAddress RemoteSession::getPeerAddress() const
{
    if(mPeerConn)
        return mPeerConn->peerAddress();
    return QHostAddress();
}

void RemoteSession::onConnected(PeerConnection *conn)
{
    Q_ASSERT(!mPeerConn);
    mPeerConn = conn;
    mPeerConn->setRemoteSession(this);

    if(mPeerConn->side() == PeerConnection::Side::Server)
    {
        sendBridgesToPeer();
    }

    sendReplicaList();

    remoteMgr()->remoteSessionsModel()->updateSessionStatus();
}

void RemoteSession::onDisconnected()
{
    Q_ASSERT(mPeerConn);
    mPeerConn->setRemoteSession(nullptr);
    mPeerConn = nullptr;

    for(RemoteCircuitBridge *bridge : std::as_const(mBridges))
    {
        bridge->mPeerNodeId = 0;
        bridge->onRemoteDisconnected();
    }

    for(const ReplicaData& repData : std::as_const(mReplicas))
    {
        for(AbstractSimulationObject *replica : repData.objects)
        {
            replica->setReplicaMode(false);
        }
    }

    remoteMgr()->replicaMgr()->removeSourceObjects(this);

    // If a session disconnects, ensure we are discoverable again
    remoteMgr()->setDiscoveryEnabled(true);

    remoteMgr()->remoteSessionsModel()->updateSessionStatus();
}

void RemoteSession::sendBridgesStatusToPeer()
{
    if(!mPeerConn)
        return;

    // TODO: maybe make a single big message with a list of all bridges
    for(RemoteCircuitBridge *bridge : std::as_const(mBridges))
    {
        if(bridge->mPeerNodeId == 0)
            continue;

        bridge->onRemoteStarted();
    }
}

void RemoteSession::sendBridgesToPeer()
{
    if(!mPeerConn)
        return;

    QCborMap map;

    quint64 localId = 0;
    for(RemoteCircuitBridge *bridge : std::as_const(mBridges))
    {
        localId++;

        QCborArray arr;
        arr.append(bridge->name());
        arr.append(bridge->peerNodeName());
        map.insert(localId, arr);
    }

    mPeerConn->sendCustonMsg(PeerConnection::BridgeList, map);
}

bool RemoteSession::isRemoteBridgeNameAvailable(const QString &name,
                                                RemoteCircuitBridge *excluded) const
{
    for(RemoteCircuitBridge *bridge : std::as_const(mBridges))
    {
        if(excluded && excluded == bridge)
            continue; // Skip self

        if(bridge->peerNodeName() == name)
            return false;
    }

    return true;
}

void RemoteSession::onRemoteBridgeResponseReceived(const BridgeResponse &msg)
{
    if(!mPeerConn)
        return;

    for(quint64 localId : msg.failedIds)
    {
        RemoteCircuitBridge *bridge = mBridges.at(localId - 1);
        if(!bridge)
            continue;

        bridge->mPeerNodeId = 0;
        bridge->onRemoteDisconnected();
    }

    if(mPeerConn->side() == PeerConnection::Side::Client)
    {
        // Client side has finished, send bridge initial status
        sendBridgesStatusToPeer();
    }
}

void RemoteSession::onRemoteBridgeListReceived(const QVector<BridgeListItem> &list)
{
    if(!mPeerConn)
        return;

    QCborArray failedIds;
    QCborMap map;

    for(const BridgeListItem& item : list)
    {
        RemoteCircuitBridge *obj = nullptr;
        for(RemoteCircuitBridge *bridge : mBridges)
        {
            if(bridge->peerNodeName() == item.localNodeName)
            {
                obj = bridge;
                break;
            }
        }

        if(!obj)
        {
            failedIds.append(qint64(item.peerNodeId));
            continue;
        }

        obj->mPeerNodeId = item.peerNodeId;
    }

    QCborArray msg;
    msg.append(failedIds);
    mPeerConn->sendCustonMsg(PeerConnection::BridgeResponse, msg);

    if(mPeerConn->side() == PeerConnection::Side::Client)
    {
        sendBridgesToPeer();
    }
    else
    {
        // Server side has finished, send initial state
        sendBridgesStatusToPeer();
    }
}

void RemoteSession::onRemoteBridgeModeChanged(quint64 localNodeId,
                                              qint8 mode, qint8 pole,
                                              qint8 replyToMode, quint8 circuitFlags)
{
    RemoteCircuitBridge *bridge = mBridges.at(localNodeId - 1);
    if(bridge)
        bridge->onRemoteNodeModeChanged(mode, pole, replyToMode, circuitFlags);
}

void RemoteSession::onLocalBridgeModeChanged(quint64 peerNodeId, qint8 mode,
                                             qint8 pole, qint8 replyToMode, quint8 circuitFlags)
{
    if(!mPeerConn)
        return;

    mPeerConn->sendBridgeStatus(peerNodeId, mode, pole, replyToMode, circuitFlags);
}

void RemoteSession::sendReplicaList()
{
    if(!mPeerConn)
        return;

    QCborArray msg;
    for(const ReplicaData& repData : mReplicas)
    {
        QCborArray objTypePair;
        objTypePair.append(repData.name);
        objTypePair.append(repData.objects.first()->getType());
        msg.append(objTypePair);
    }

    mPeerConn->sendCustonMsg(PeerConnection::ReplicaList, msg);
}

void RemoteSession::onReplicaListReceived(const QCborArray &msg)
{
    if(!mPeerConn)
        return;

    ModeManager *modeMgr = remoteMgr()->modeMgr();
    ReplicaObjectManager *replicaMgr = remoteMgr()->replicaMgr();

    QCborArray failedIds;

    quint64 replicaId = 0;
    for(const QCborValue& val : msg)
    {
        if(!val.isArray())
        {
            failedIds.append(qint64(replicaId++));
            continue;
        }

        QCborArray objTypePair = val.toArray();
        if(objTypePair.size() != 2 || !objTypePair.at(0).isString() || !objTypePair.at(1).isString())
        {
            failedIds.append(qint64(replicaId++));
            continue;
        }

        const QString objName = objTypePair.at(0).toString();
        const QString objType = objTypePair.at(1).toString();

        AbstractSimulationObjectModel *objModel = modeMgr->modelForType(objType);
        if(!objModel)
        {
            failedIds.append(qint64(replicaId++));
            continue;
        }

        AbstractSimulationObject *sourceObj = objModel->getObjectByName(objName);
        if(!sourceObj)
        {
            failedIds.append(qint64(replicaId++));
            continue;
        }

        replicaMgr->addSourceObject(sourceObj, this, replicaId);
        replicaId++;
    }

    mPeerConn->sendCustonMsg(PeerConnection::ReplicaResponse, failedIds);
}

void RemoteSession::onReplicaResponseReceived(const QCborArray &msg)
{
    QVector<quint64> failedIds;
    failedIds.reserve(msg.size());

    for(const QCborValue& val : msg)
        failedIds.append(val.toInteger());

    for(quint64 replicaId = 0; replicaId < quint64(mReplicas.size()); replicaId++)
    {
        if(failedIds.contains(replicaId))
            continue;

        const ReplicaData& repData = mReplicas.at(replicaId);
        for(AbstractSimulationObject *replica : repData.objects)
        {
            replica->setReplicaMode(true);
        }
    }
}

void RemoteSession::sendSourceObjectState(quint64 objectId, const QCborMap &objState)
{
    if(!mPeerConn)
        return;

    QCborArray msg;
    msg.append(qint64(objectId));
    msg.append(objState);
    mPeerConn->sendCustonMsg(PeerConnection::ReplicaStatus, msg);
}

void RemoteSession::onSourceObjectStateReceived(quint64 replicaId, const QCborMap &objState)
{
    if(replicaId >= quint64(mReplicas.size()))
        return;

    const ReplicaData& repData = mReplicas.at(replicaId);
    for(AbstractSimulationObject *replica : repData.objects)
    {
        replica->setReplicaState(objState);
    }
}

void RemoteSession::addReplica(AbstractSimulationObject *replicaObj, const QString &name)
{
    auto replicaIt = std::find_if(mReplicas.begin(),
                                  mReplicas.end(),
                                  [replicaObj, name](const ReplicaData& repData) -> bool
    {
        return repData.name == name && replicaObj->getType() == repData.objects.first()->getType();
    });

    if(replicaIt == mReplicas.end())
        replicaIt = mReplicas.insert(mReplicas.size(), {name, {}});

    Q_ASSERT(!replicaIt->objects.contains(replicaObj));
    replicaIt->objects.append(replicaObj);
}

void RemoteSession::removeReplica(AbstractSimulationObject *replicaObj, const QString &name)
{
    auto replicaIt = std::find_if(mReplicas.begin(),
                                  mReplicas.end(),
                                  [replicaObj, name](const ReplicaData& repData) -> bool
    {
        // NOTE: we cannot access getType() if replicaObj is being destroyed
        // So use contains instead
        return repData.name == name && repData.objects.contains(replicaObj);
    });

    Q_ASSERT(replicaIt != mReplicas.end());
    Q_ASSERT(replicaIt->objects.contains(replicaObj));

    replicaIt->objects.removeOne(replicaObj);
    if(replicaIt->objects.isEmpty())
        mReplicas.erase(replicaIt);
}
