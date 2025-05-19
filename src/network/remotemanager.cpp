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

#include "remotesession.h"
#include "remotesessionsmodel.h"
#include "replicaobjectmanager.h"

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
    mRemoteSessionsModel = new RemoteSessionsModel(this);
    mReplicaMgr = new ReplicaObjectManager(this);
}

RemoteManager::~RemoteManager()
{   
    mPeerClient->setCommunicationEnabled(false);

    clear();

    delete mRemoteSessionsModel;
    mRemoteSessionsModel = nullptr;

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

void RemoteManager::clear()
{
    setSessionName(QString());

    const auto remoteSessions = mRemoteSessions;
    for(RemoteSession *r : remoteSessions)
        removeRemoteSession(r);
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

    if(fromName == toName)
        return false;

    RemoteSession *remoteSession = mRemoteSessions.value(fromName);

    if(!remoteSession || mRemoteSessions.contains(toName))
        return false;

    mRemoteSessions.remove(fromName);
    mRemoteSessions.insert(toName, remoteSession);

    mRemoteSessionsModel->sortItems();

    return true;
}

ReplicaObjectManager *RemoteManager::replicaMgr() const
{
    return mReplicaMgr;
}

RemoteSessionsModel *RemoteManager::remoteSessionsModel() const
{
    return mRemoteSessionsModel;
}

RemoteSession *RemoteManager::addRemoteSession(const QString &sessionName)
{
    const QString trimmedName = sessionName.trimmed();

    auto it = mRemoteSessions.constFind(trimmedName);
    if(it != mRemoteSessions.constEnd())
        return it.value();

    RemoteSession *remoteSession = new RemoteSession(trimmedName, this);
    mRemoteSessions.insert(trimmedName, remoteSession);

    mRemoteSessionsModel->addRemoteSession(remoteSession);

    return remoteSession;
}

RemoteSession *RemoteManager::getRemoteSession(const QString &sessionName) const
{
    return mRemoteSessions.value(sessionName);
}

void RemoteManager::removeRemoteSession(RemoteSession *remoteSession)
{
    if(!remoteSession)
        return;

    mRemoteSessions.remove(remoteSession->getSessionName());
    mRemoteSessionsModel->removeRemoteSession(remoteSession);
    delete remoteSession;
}

void RemoteManager::addConnection(PeerConnection *conn)
{
    RemoteSession *remoteSession = mRemoteSessions.value(conn->sessionName());
    if(remoteSession)
        remoteSession->onConnected(conn);
}
