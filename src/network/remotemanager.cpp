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

#include "remotesession.h"

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

    RemoteSession *remoteSession = mRemoteSessions.take(fromConnId);

    if(!remoteSession || mRemoteSessions.contains(toConnId))
        return false;

    mRemoteSessions.insert(toConnId, remoteSession);

    return true;
}

void RemoteManager::addRemoteBridge(RemoteCircuitBridge *bridge, const QString &peerSession)
{
    RemoteSession *remoteSession = addRemoteSession(peerSession);
    remoteSession->addRemoteBridge(bridge);
}

void RemoteManager::removeRemoteBridge(RemoteCircuitBridge *bridge, const QString &peerSession)
{
    RemoteSession *remoteSession = mRemoteSessions.value(qHash(peerSession));
    if(!remoteSession)
        return;

    remoteSession->removeRemoteBridge(bridge);
}

void RemoteManager::onLocalBridgeModeChanged(quint64 peerSessionId, quint64 peerNodeId,
                                             qint8 mode, qint8 pole, qint8 replyToMode)
{
    RemoteSession *remoteSession = mRemoteSessions.value(peerSessionId);
    if(!remoteSession)
        return;

    remoteSession->getConnection()->sendBridgeStatus(peerNodeId, mode, pole, replyToMode);
}

RemoteSession *RemoteManager::addRemoteSession(const QString &sessionName)
{
    const qint64 sessionId = qHash(sessionName);
    auto it = mRemoteSessions.constFind(sessionId);
    if(it != mRemoteSessions.constEnd())
        return it.value();

    RemoteSession *remoteSession = new RemoteSession(sessionName, this);
    mRemoteSessions.insert(sessionId, remoteSession);
    return remoteSession;
}

void RemoteManager::removeRemoteSession(const QString &sessionName)
{
    RemoteSession *remoteSession = mRemoteSessions.take(qHash(sessionName));
    if(remoteSession)
        delete remoteSession;
}

void RemoteManager::addConnection(PeerConnection *conn)
{
    RemoteSession *remoteSession = mRemoteSessions.value(conn->getHashedSessionName());
    if(remoteSession)
        remoteSession->onConnected(conn);

    if(conn->side() == PeerConnection::Side::Server)
    {
        remoteSession->sendBridgesTo(conn);
    }
}

void RemoteManager::removeConnection(PeerConnection *conn)
{
    RemoteSession *remoteSession = mRemoteSessions.value(conn->getHashedSessionName());
    if(!remoteSession)
        return;

    remoteSession->onDisconnected();
}
