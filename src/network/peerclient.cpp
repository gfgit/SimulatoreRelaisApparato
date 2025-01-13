// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "remotemanager.h"
#include "peerclient.h"
#include "peerconnection.h"
#include "peermanager.h"

#include "../views/modemanager.h"

#include <QHostInfo>

PeerClient::PeerClient(RemoteManager *mgr)
    : QObject(mgr)
{
    peerManager = new PeerManager(this, mgr);

    connect(peerManager, &PeerManager::newConnection,
            this, &PeerClient::newConnection);
    connect(&server, &PeerServer::newConnection,
            this, &PeerClient::newConnection);

    connect(peerManager, &PeerManager::sessionNameChanged,
            this, &PeerClient::nickNameChanged);
}

void PeerClient::sendMessage(const QString &message)
{
    if (message.isEmpty())
        return;

    for (PeerConnection *connection : std::as_const(peers))
        connection->sendMessage(message);
}

QString PeerClient::nickName() const
{
    return peerManager->sessionName() + '@' + QHostInfo::localHostName()
           + ':' + QString::number(server.serverPort());
}

bool PeerClient::hasConnection(const QByteArray &peerUniqueId, const QString& sessionName) const
{
    if(peers.contains(peerUniqueId))
        return true;

    for(PeerConnection *conn : std::as_const(peers))
    {
        if(conn->sessionName() == sessionName)
            return true;
    }

    return false;
}

void PeerClient::setCommunicationEnabled(bool val)
{
    if(peerManager->modeMgr()->mode() != FileMode::Simulation
            || peerManager->sessionName().isEmpty())
        val = false; // Can communicate only during simulation

    if(val == mEnabled)
        return;

    mEnabled = val;

    if(!mEnabled)
    {
        // Cannot broadcast if server is not running
        peerManager->stopBroadcasting();

        // Close active connections
        const auto peersCopy = peers;
        for(PeerConnection *conn : peersCopy)
            removeConnection(conn);
    }

    server.setEnabled(mEnabled);

    // Update nickname port
    emit nickNameChanged();

    emit enabledChanged();

    emit peerManager->remoteMgr()->networkStateChanged();
}

bool PeerClient::isCommunicationEnabled() const
{
    return mEnabled;
}

void PeerClient::newConnection(PeerConnection *connection)
{
    connection->setGreetingMessage(peerManager->sessionName(), peerManager->uniqueId());
    connect(connection, &PeerConnection::readyForUse, this, &PeerClient::readyForUse);
    connect(connection, &PeerConnection::errorOccurred, connection, &QObject::deleteLater);
    connect(connection, &PeerConnection::disconnected, connection, &QObject::deleteLater);
}

void PeerClient::readyForUse()
{
    PeerConnection *connection = qobject_cast<PeerConnection *>(sender());
    if (!connection || hasConnection(connection->uniqueId(), connection->sessionName()))
    {
        if (connection) {
            connection->disconnectFromHost();
            connection->deleteLater();
        }
        return;
    }

    connect(connection, &PeerConnection::errorOccurred, this, &PeerClient::connectionError);
    connect(connection, &PeerConnection::disconnected, this, &PeerClient::disconnected);
    connect(connection, &PeerConnection::newMessage, this, &PeerClient::newMessage);

    peers.insert(connection->uniqueId(), connection);
    QString nick = connection->nickName();
    if (!nick.isEmpty())
        emit newParticipant(nick, connection->side() == PeerConnection::Side::Server);

    peerManager->remoteMgr()->addConnection(connection);
}

void PeerClient::onSessionNameChanged()
{
    emit nickNameChanged();
}

void PeerClient::disconnected()
{
    if (PeerConnection *connection = qobject_cast<PeerConnection *>(sender()))
        removeConnection(connection);
}

void PeerClient::connectionError(QAbstractSocket::SocketError /* socketError */)
{
    if (PeerConnection *connection = qobject_cast<PeerConnection *>(sender()))
        removeConnection(connection);
}

void PeerClient::removeConnection(PeerConnection *connection)
{
    if (peers.remove(connection->uniqueId()))
    {
        peerManager->remoteMgr()->removeConnection(connection);
        QString nick = connection->nickName();
        if (!nick.isEmpty())
            emit participantLeft(nick);
    }
    connection->deleteLater();
}
