/**
 * src/network/peermanager.cpp
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
// Copyright (C) 2018 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "remotemanager.h"
#include "peerclient.h"
#include "peerconnection.h"
#include "peermanager.h"

#include "../views/modemanager.h"

#include <QNetworkInterface>
#include <QUuid>

#include "info.h"

static const qint32 BroadcastInterval = 2000;
static const unsigned broadcastPort = 45000;

PeerManager::PeerManager(PeerClient *client, RemoteManager *mgr)
    : QObject(client)
    , mRemoteMgr(mgr)
    , mClient(client)
{
    // We generate a unique per-process identifier so we can avoid multiple
    // connections to/from the same remote peer as well as ignore our own
    // broadcasts.
    localUniqueId = QUuid::createUuid().toByteArray();

    updateAddresses();

    connect(&broadcastSocket, &QUdpSocket::readyRead,
            this, &PeerManager::readBroadcastDatagram);

    broadcastTimer.setInterval(BroadcastInterval);
    connect(&broadcastTimer, &QTimer::timeout,
            this, &PeerManager::sendBroadcastDatagram);
}

void PeerManager::setServerPort(int port)
{
    serverPort = port;
}

QString PeerManager::sessionName() const
{
    return localSessionName;
}

void PeerManager::setSessionName(const QString &str)
{
    if(mClient->isCommunicationEnabled())
        return; // Name can be changed only when offline

    QString newName = str.simplified();
    if(localSessionName == newName)
        return;

    localSessionName = newName;

    emit sessionNameChanged(newName);
}

QByteArray PeerManager::uniqueId() const
{
    return localUniqueId;
}

void PeerManager::startBroadcasting()
{
    setDiscoveryEnabled(true);
}

void PeerManager::stopBroadcasting()
{
    setDiscoveryEnabled(false);
}

bool PeerManager::isLocalHostAddress(const QHostAddress &address) const
{
    return ipAddresses.contains(address);
}

void PeerManager::sendBroadcastDatagram()
{
    if(!mEnabled)
        return;

    QByteArray datagram;
    {
        QCborStreamWriter writer(&datagram);
        writer.startArray(4);
        writer.append(AppVersion);
        writer.append(localSessionName);
        writer.append(localUniqueId);
        writer.append(serverPort);
        writer.endArray();
    }

    bool validBroadcastAddresses = true;
    for (const QHostAddress &address : std::as_const(broadcastAddresses))
    {
        if (broadcastSocket.writeDatagram(datagram, address, broadcastPort) == -1)
            validBroadcastAddresses = false;
    }

    if (!validBroadcastAddresses)
        updateAddresses();
}

void PeerManager::readBroadcastDatagram()
{
    while (broadcastSocket.hasPendingDatagrams())
    {
        QHostAddress senderIp;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(broadcastSocket.pendingDatagramSize());
        if (broadcastSocket.readDatagram(datagram.data(), datagram.size(),
                                         &senderIp, &senderPort) == -1)
            continue;

        if(!mEnabled)
            continue; // Read all but ignore contents

        QString peerAppVersion;
        QString peerSessionName;
        QByteArray peerUniqueId;
        int senderServerPort;
        {
            // decode the datagram
            QCborStreamReader reader(datagram);
            if (reader.lastError() != QCborError::NoError || !reader.isArray())
                continue;
            if (!reader.isLengthKnown() || reader.length() != 4)
                continue;

            reader.enterContainer();
            if (reader.lastError() != QCborError::NoError || !reader.isString())
                continue;

            auto strResult = reader.readString();
            while (strResult.status == QCborStreamReader::Ok)
            {
                peerAppVersion = strResult.data;
                strResult = reader.readString();
            }

            strResult = reader.readString();
            while (strResult.status == QCborStreamReader::Ok)
            {
                peerSessionName = strResult.data;
                strResult = reader.readString();
            }

            auto r = reader.readByteArray();
            while (r.status == QCborStreamReader::Ok)
            {
                peerUniqueId = r.data;
                r = reader.readByteArray();
            }

            if (reader.lastError() != QCborError::NoError || !reader.isUnsignedInteger())
                continue;
            senderServerPort = reader.toInteger();
        }

        if (peerAppVersion != AppVersion
                || peerUniqueId == localUniqueId
                || peerSessionName == localSessionName)
            continue;

        // Auto-connect only to interesting sessions
        if(!mRemoteMgr->isSessionReferenced(peerSessionName))
            continue;

        if (!mClient->hasConnection(peerUniqueId, peerSessionName))
        {
            PeerConnection *connection = new PeerConnection(this);
            connection->setSide(PeerConnection::Side::Client);
            emit newConnection(connection);
            connection->connectToHost(senderIp, senderServerPort);
        }
    }
}

void PeerManager::updateAddresses()
{
    broadcastAddresses.clear();
    ipAddresses.clear();
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces)
    {
        const QList<QNetworkAddressEntry> entries = interface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries)
        {
            QHostAddress broadcastAddress = entry.broadcast();
            if (broadcastAddress != QHostAddress::Null && entry.ip() != QHostAddress::LocalHost)
            {
                broadcastAddresses << broadcastAddress;
                ipAddresses << entry.ip();
            }
        }
    }
}

bool PeerManager::isDiscoveryEnabled() const
{
    return mEnabled;
}

void PeerManager::setDiscoveryEnabled(bool newEnabled)
{
    if(mRemoteMgr->modeMgr()->mode() != FileMode::Simulation || localSessionName.isEmpty())
        newEnabled = false; // Can discover only during simulation

    if(mEnabled == newEnabled)
        return;

    mEnabled = newEnabled;

    if(mEnabled)
    {
        // Start broadcasting

        // Ensure server is running
        mClient->setCommunicationEnabled(true);

        setServerPort(mClient->getServerPort());

        broadcastSocket.bind(QHostAddress::Any, broadcastPort, QUdpSocket::ShareAddress
                             | QUdpSocket::ReuseAddressHint);

        broadcastTimer.start();
    }
    else
    {
        // Stop broadcasting

        broadcastTimer.stop();

        broadcastSocket.close();
    }

    emit enabledChanged();

    emit mRemoteMgr->networkStateChanged();
}

ModeManager *PeerManager::modeMgr() const
{
    return mRemoteMgr->modeMgr();
}
