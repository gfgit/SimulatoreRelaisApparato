/**
 * src/network/peerclient.h
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

// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PEER_CLIENT_H
#define PEER_CLIENT_H

#include "peerserver.h"

#include <QAbstractSocket>
#include <QHash>
#include <QHostAddress>

class RemoteManager;
class PeerManager;

class PeerClient : public QObject
{
    Q_OBJECT

public:
    PeerClient(RemoteManager *mgr);

    void sendMessage(const QString &message);
    QString nickName() const;
    bool hasConnection(const QByteArray &peerUniqueId, const QString &sessionName) const;

    void setCommunicationEnabled(bool val);
    bool isCommunicationEnabled() const;

    inline int getServerPort() const
    {
        return server.serverPort();
    }

    inline PeerManager *getPeerManager() const
    {
        return peerManager;
    }

signals:
    void newMessage(const QString &from, const QString &message);
    void newParticipant(const QString &nick, bool server);
    void participantLeft(const QString &nick);

    void nickNameChanged();

    void enabledChanged();

private slots:
    void newConnection(PeerConnection *connection);
    void connectionError(QAbstractSocket::SocketError socketError);
    void disconnected();
    void readyForUse();

    void onSessionNameChanged();

private:
    void removeConnection(PeerConnection *connection);

    PeerManager *peerManager;
    PeerServer server;
    QHash<QByteArray, PeerConnection *> peers;

    bool mEnabled = false;
};

#endif // PEER_CLIENT_H
