/**
 * src/network/peermanager.h
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

#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

class ModeManager;
class RemoteManager;
class PeerClient;
class PeerConnection;

class PeerManager : public QObject
{
    Q_OBJECT

public:
    explicit PeerManager(PeerClient *client, RemoteManager *mgr);

    void setServerPort(int port);

    QString sessionName() const;
    void setSessionName(const QString& str);

    QByteArray uniqueId() const;

    void startBroadcasting();
    void stopBroadcasting();

    bool isLocalHostAddress(const QHostAddress &address) const;

    bool isDiscoveryEnabled() const;
    void setDiscoveryEnabled(bool newEnabled);

    ModeManager *modeMgr() const;

    inline RemoteManager *remoteMgr() const
    {
        return mRemoteMgr;
    }

    void updateAddresses();

signals:
    void sessionNameChanged(const QString& newName);

    void newConnection(PeerConnection *connection);

    void enabledChanged();

private slots:
    void sendBroadcastDatagram();
    void readBroadcastDatagram();

private:
    RemoteManager *mRemoteMgr = nullptr;
    PeerClient *mClient = nullptr;
    QList<QHostAddress> broadcastAddresses;
    QList<QHostAddress> ipAddresses;
    QUdpSocket broadcastSocket;
    QTimer broadcastTimer;

    QString localSessionName;
    QByteArray localUniqueId;

    int serverPort = 0;

    bool mEnabled = false;
};

#endif
