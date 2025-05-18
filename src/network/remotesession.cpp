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

#include "remotemanager.h"
#include "peerconnection.h"

#include "../objects/circuit_bridge/remotecircuitbridge.h"

#include <QCborMap>
#include <QCborArray>

RemoteSession::RemoteSession(const QString &sessionName, RemoteManager *remoteMgr)
    : QObject{remoteMgr}
    , mSessionName(sessionName)
{

}

RemoteSession::~RemoteSession()
{
    const auto bridges = mBridges;
    for(RemoteCircuitBridge *bridge : bridges)
    {
        bridge->setRemoteSession(nullptr);
    }
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

void RemoteSession::onConnected(PeerConnection *conn)
{
    Q_ASSERT(!mPeerConn);
    mPeerConn = conn;
    mPeerConn->setRemoteSession(this);

    if(mPeerConn->side() == PeerConnection::Side::Server)
    {
        sendBridgesToPeer();
    }
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

bool RemoteSession::isRemoteBridgeNameAvailable(const QString &name) const
{
    for(RemoteCircuitBridge *bridge : std::as_const(mBridges))
    {
        if(bridge->peerNodeName() == name)
            return false;
    }

    return true;
}

void RemoteSession::onRemoteBridgeResponseReceived(PeerConnection *conn, const BridgeResponse &msg)
{
    for(quint64 localId : msg.failedIds)
    {
        RemoteCircuitBridge *bridge = mBridges.at(localId - 1);
        if(!bridge)
            continue;

        bridge->mPeerNodeId = 0;
        bridge->onRemoteDisconnected();
    }

    for(auto m = msg.newMappings.cbegin(), end = msg.newMappings.cend(); m != end; m++)
    {
        RemoteCircuitBridge *bridge = mBridges.at(m.key() - 1);
        if(!bridge)
            continue;

        bridge->mPeerNodeId = m.value();
    }

    if(conn->side() == PeerConnection::Side::Client)
    {
        // Client side has finished, send bridge initial status
        sendBridgesStatusToPeer();
    }
}

void RemoteSession::onRemoteBridgeListReceived(PeerConnection *conn, const QVector<BridgeListItem> &list)
{
    QCborArray failedIds;
    QCborMap map;

    for(const BridgeListItem& item : list)
    {
        RemoteCircuitBridge *obj = nullptr;
        for(RemoteCircuitBridge *bridge : mBridges)
        {
            if(bridge->name() == item.localNodeName)
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
    msg.append(map);
    conn->sendCustonMsg(PeerConnection::BridgeResponse, msg);

    if(conn->side() == PeerConnection::Side::Client)
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
                                              qint8 mode, qint8 pole, qint8 replyToMode)
{
    RemoteCircuitBridge *bridge = mBridges.at(localNodeId - 1);
    if(bridge)
        bridge->onRemoteNodeModeChanged(mode, pole, replyToMode);
}

void RemoteSession::onLocalBridgeModeChanged(quint64 peerNodeId, qint8 mode, qint8 pole, qint8 replyToMode)
{
    mPeerConn->sendBridgeStatus(peerNodeId, mode, pole, replyToMode);
}
