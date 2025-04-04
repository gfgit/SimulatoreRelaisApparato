/**
 * src/network/remotemanager.cpp
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

#include "remotemanager.h"

#include "peerclient.h"
#include "peermanager.h"
#include "peerconnection.h"

#include "../objects/abstractsimulationobjectmodel.h"

#include "../views/modemanager.h"
#include "../objects/circuit_bridge/remotecircuitbridge.h"

#include <QCborMap>
#include <QCborArray>

/* Protocol
 *
 * Server:
 * - Send bridges list
 * - Wait response
 * - Wait client bridge list
 * - Send response
 * - Send bridge initial status
 *
 * Client:
 * - Wait client bridge list
 * - Send response
 * - Send bridges list
 * - Wait response
 * - Send bridge initial status
 */

RemoteManager::RemoteManager(ModeManager *mgr)
    : QObject(mgr)
{
    mPeerClient = new PeerClient(this);
    mPeerManager = mPeerClient->getPeerManager();
}

RemoteManager::~RemoteManager()
{
    mPeerClient->setCommunicationEnabled(false);

    delete mPeerClient;
    mPeerClient = nullptr;
    mPeerManager = nullptr;
}

ModeManager *RemoteManager::modeMgr() const
{
    return static_cast<ModeManager *>(parent());
}

QString RemoteManager::sessionName() const
{
    return mPeerManager->sessionName();
}

void RemoteManager::setSessionName(const QString &newSessionName)
{
    mPeerManager->setSessionName(newSessionName);
    modeMgr()->setFileEdited();
}

void RemoteManager::setOnline(bool val)
{
    mPeerClient->setCommunicationEnabled(val);
    if(val)
        setDiscoveryEnabled(true);
}

bool RemoteManager::isOnline() const
{
    return mPeerClient->isCommunicationEnabled();
}

void RemoteManager::setDiscoveryEnabled(bool val)
{
    if(!mPeerClient->isCommunicationEnabled())
        val = false;
    mPeerManager->setDiscoveryEnabled(val);
}

bool RemoteManager::isDiscoveryEnabled() const
{
    return mPeerManager->isDiscoveryEnabled();
}

void RemoteManager::refreshNetworkAddresses()
{
    mPeerManager->updateAddresses();
}

bool RemoteManager::renameRemoteSession(const QString &fromName, const QString &toName)
{
    if(isOnline())
        return false;

    const quint64 fromConnId = qHash(fromName);
    const quint64 toConnId = qHash(toName);

    if(fromConnId == toConnId)
        return false;

    if(!mRemoteBridges.contains(fromConnId) || mRemoteBridges.contains(toConnId))
        return false;

    const auto vec = mRemoteBridges.take(fromConnId);

    for(RemoteCircuitBridge *bridge : vec)
    {
        if(!bridge->remoteSessionName().isEmpty())
            bridge->onRemoteSessionRenamed(toName);
    }

    mRemoteBridges.insert(toConnId, vec);

    return true;
}

void RemoteManager::addRemoteBridge(RemoteCircuitBridge *bridge, const QString &peerSession)
{
    const quint64 peerConnId = qHash(peerSession);

    auto it = mRemoteBridges.find(peerConnId);
    if(it == mRemoteBridges.end())
        it = mRemoteBridges.insert(peerConnId, {});

    it.value().append(bridge);
}

void RemoteManager::removeRemoteBridge(RemoteCircuitBridge *bridge, const QString &peerSession)
{
    const quint64 peerConnId = qHash(peerSession);

    auto it = mRemoteBridges.find(peerConnId);
    Q_ASSERT(it != mRemoteBridges.end());

    it.value().removeOne(bridge);
    if(it.value().isEmpty())
        mRemoteBridges.erase(it);
}

void RemoteManager::onLocalBridgeModeChanged(quint64 peerSessionId, quint64 peerNodeId,
                                             qint8 mode, qint8 pole, qint8 replyToMode)
{
    PeerConnection *conn = mConnections.value(peerSessionId, nullptr);
    if(!conn)
        return;

    conn->sendBridgeStatus(peerNodeId, mode, pole, replyToMode);
}

void RemoteManager::onRemoteBridgeModeChanged(quint64 peerSessionId, quint64 localNodeId,
                                              qint8 mode, qint8 pole, qint8 replyToMode)
{
    PeerConnection *conn = mConnections.value(peerSessionId, nullptr);
    if(!conn)
        return;

    auto it = mRemoteBridges.find(conn->getHashedSessionName());
    if(it == mRemoteBridges.end())
        return;

    RemoteCircuitBridge *bridge = it.value().value(localNodeId - 1, nullptr);
    if(bridge)
        bridge->onRemoteNodeModeChanged(mode, pole, replyToMode);
}

void RemoteManager::onRemoteBridgeListReceived(PeerConnection *conn, const QVector<BridgeListItem> &list)
{
    auto it = mRemoteBridges.find(conn->getHashedSessionName());
    if(it == mRemoteBridges.end())
    {
        // Create new connection bridge list
        it = mRemoteBridges.insert(conn->getHashedSessionName(), {});
    }

    QCborArray failedIds;
    QCborMap map;

    for(const BridgeListItem& item : list)
    {
        RemoteCircuitBridge *obj = nullptr;
        for(RemoteCircuitBridge *bridge : it.value())
        {
            if(bridge->name() == item.localNodeName)
            {
                obj = bridge;
                break;
            }
        }

        if(!obj)
        {
            auto model = modeMgr()->modelForType(RemoteCircuitBridge::Type);
            RemoteCircuitBridge *candidate = static_cast<RemoteCircuitBridge *>(model->getObjectByName(item.localNodeName));
            if(candidate && candidate->isRemote()
                    && (candidate->mPeerSessionId == 0 || candidate->mPeerSessionId == conn->getHashedSessionName()))
            {
                obj = candidate;
            }
        }

        if(!obj)
        {
            failedIds.append(qint64(item.peerNodeId));
            continue;
        }

        obj->mPeerNodeId = item.peerNodeId;
        obj->mPeerNodeName = item.peerNodeName;

        if(!obj->mPeerSessionId)
        {
            obj->mPeerSessionId = conn->getHashedSessionName();
            map.insert(item.peerNodeId, it.value().size());
            it.value().append(obj);
        }
    }

    QCborArray msg;
    msg.append(failedIds);
    msg.append(map);
    conn->sendCustonMsg(PeerConnection::BridgeResponse, msg);

    if(conn->side() == PeerConnection::Side::Client)
    {
        sendBridgesTo(conn);
    }
    else
    {
        // Server side has finished, send initial state
        sendBridgesStatusTo(conn);
    }
}

void RemoteManager::onRemoteBridgeResponseReceived(PeerConnection *conn, const BridgeResponse &msg)
{
    auto it = mRemoteBridges.find(conn->getHashedSessionName());
    if(it == mRemoteBridges.end())
        return;

    for(quint64 localId : msg.failedIds)
    {
        RemoteCircuitBridge *bridge = it.value().value(localId - 1, nullptr);
        if(!bridge)
            continue;

        bridge->mPeerSessionId = 0;
        bridge->mPeerNodeId = 0;
        bridge->onRemoteDisconnected();
    }

    for(auto m = msg.newMappings.cbegin(), end = msg.newMappings.cend(); m != end; m++)
    {
        RemoteCircuitBridge *bridge = it.value().value(m.key() - 1, nullptr);
        if(!bridge)
            continue;

        bridge->mPeerNodeId = m.value();
    }

    if(conn->side() == PeerConnection::Side::Client)
    {
        // Client side has finished, send bridge initial status
        sendBridgesStatusTo(conn);
    }
}

void RemoteManager::addConnection(PeerConnection *conn)
{
    conn->setRemoteMgr(this);
    const QString& remoteSessionName = conn->sessionName();
    const quint64 remoteSessionHash = conn->getHashedSessionName();

    mConnections.insert(remoteSessionHash, conn);

    auto it = mRemoteBridges.find(remoteSessionHash);
    if(it != mRemoteBridges.end())
    {
        for(RemoteCircuitBridge *bridge : it.value())
        {
            bridge->mPeerSessionId = remoteSessionHash;
        }
    }

    if(conn->side() == PeerConnection::Side::Server)
    {
        sendBridgesTo(conn);
    }
}

void RemoteManager::removeConnection(PeerConnection *conn)
{
    conn->setRemoteMgr(nullptr);
    mConnections.remove(conn->getHashedSessionName());

    auto it = mRemoteBridges.find(conn->getHashedSessionName());
    if(it != mRemoteBridges.end())
    {
        QVector<RemoteCircuitBridge *> &vec = it.value();
        auto br = vec.begin();
        while(br != vec.end())
        {
            RemoteCircuitBridge *bridge = *br;
            bridge->mPeerSessionId = 0;
            bridge->mPeerNodeId = 0;
            bridge->onRemoteDisconnected();
            if(bridge->remoteSessionName().isEmpty())
            {
                br = vec.erase(br);
                continue;
            }

            br++;
        }
    }
}

void RemoteManager::sendBridgesTo(PeerConnection *conn)
{
    auto it = mRemoteBridges.find(conn->getHashedSessionName());
    if(it == mRemoteBridges.end())
    {
        // ERROR: Send empty map
        conn->sendCustonMsg(PeerConnection::BridgeList, QCborMap{});
        return;
    }

    QCborMap map;

    quint64 localId = 0;
    for(RemoteCircuitBridge *bridge : it.value())
    {
        localId++;

        if(bridge->remoteSessionName().isEmpty())
            break;

        QCborArray arr;
        arr.append(bridge->name());
        arr.append(bridge->mPeerNodeName);
        map.insert(localId, arr);
    }

    conn->sendCustonMsg(PeerConnection::BridgeList, map);
}

void RemoteManager::sendBridgesStatusTo(PeerConnection *conn)
{
    auto it = mRemoteBridges.find(conn->getHashedSessionName());
    if(it == mRemoteBridges.end())
        return;

    // TODO: maybe make a single big message with a list of all bridges
    for(RemoteCircuitBridge *bridge : it.value())
    {
        if(bridge->mPeerSessionId == 0 || bridge->mPeerNodeId == 0)
            continue;

        bridge->onRemoteStarted();
    }
}
