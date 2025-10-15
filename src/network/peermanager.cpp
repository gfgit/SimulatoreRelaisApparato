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

#include "traintastic-simulator/traintasticsimmanager.h"

#include <QNetworkInterface>
#include <QUuid>

#include <QTimerEvent>

#include "info.h"

static const qint32 BroadcastInterval = 2000;
static const unsigned PeerBroadcastPort = 45000;
static const unsigned TraintasticBroadcastPort = 5742;

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

    connect(&peerBroadcastSocket, &QUdpSocket::readyRead,
            this, &PeerManager::readPeerBroadcastDatagram);
    connect(&traintasticBroadcastSocket, &QUdpSocket::readyRead,
            this, &PeerManager::readTraintasticBroadcastDatagram);
    connect(&traintasticBroadcastSocket, &QUdpSocket::errorOccurred,
            this, [this]()
            {
                qDebug() << "Traintastic UDP error:" << traintasticBroadcastSocket.error() << traintasticBroadcastSocket.errorString();
            });
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
    setPeerDiscoveryEnabled(true);
}

void PeerManager::stopBroadcasting()
{
    setPeerDiscoveryEnabled(false);
}

bool PeerManager::isLocalHostAddress(const QHostAddress &address) const
{
    return ipAddresses.contains(address);
}

void PeerManager::sendPeerBroadcastDatagram()
{
    if(!mPeerDiscoveryEnabled)
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
        if (peerBroadcastSocket.writeDatagram(datagram, address, PeerBroadcastPort) == -1)
            validBroadcastAddresses = false;
    }

    if (!validBroadcastAddresses || broadcastAddresses.isEmpty())
        updateAddresses();
}

void PeerManager::readPeerBroadcastDatagram()
{
    while (peerBroadcastSocket.hasPendingDatagrams())
    {
        QHostAddress senderIp;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(peerBroadcastSocket.pendingDatagramSize());
        if (peerBroadcastSocket.readDatagram(datagram.data(), datagram.size(),
                                             &senderIp, &senderPort) == -1)
            continue;

        if(!mPeerDiscoveryEnabled)
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

void PeerManager::sendTraintasticBroadcastDatagram()
{
    if(!mTraintasticDiscoveryEnabled)
        return;

    QByteArray datagram("sim?");

    bool validBroadcastAddresses = true;
    for (const QHostAddress &address : std::as_const(broadcastAddresses))
    {
        if (traintasticBroadcastSocket.writeDatagram(datagram, address, TraintasticBroadcastPort) == -1)
            validBroadcastAddresses = false;
    }

    if (!validBroadcastAddresses || broadcastAddresses.isEmpty())
        updateAddresses();
}

void PeerManager::readTraintasticBroadcastDatagram()
{
    while (traintasticBroadcastSocket.hasPendingDatagrams())
    {
        QHostAddress senderIp;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(traintasticBroadcastSocket.pendingDatagramSize());
        if (traintasticBroadcastSocket.readDatagram(datagram.data(), datagram.size(),
                                             &senderIp, &senderPort) == -1)
            continue;

        if(!mTraintasticDiscoveryEnabled)
            continue; // Read all but ignore contents

        if(datagram.startsWith("sim?"))
            continue;

        if(datagram.size() != 6 || !datagram.startsWith("sim!"))
            continue; // Invalid response

        quint16 traintasticServerPort = *reinterpret_cast<const quint16 *>(datagram.constData() + 4);
        traintasticServerPort = qFromBigEndian(traintasticServerPort);

        modeMgr()->getTraitasticSimMgr()->tryConnectToServer(senderIp, traintasticServerPort);
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
            if (broadcastAddress != QHostAddress::Null)
            {
                broadcastAddresses << broadcastAddress;
                ipAddresses << entry.ip();
            }
        }
    }
}

void PeerManager::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == peerBroadcastTimer.timerId())
    {
        sendPeerBroadcastDatagram();
        return;
    }

    if(ev->timerId() == traintasticBroadcastTimer.timerId())
    {
        sendTraintasticBroadcastDatagram();
        return;
    }

    QObject::timerEvent(ev);
}

bool PeerManager::isPeerDiscoveryEnabled() const
{
    return mPeerDiscoveryEnabled;
}

void PeerManager::setPeerDiscoveryEnabled(bool newEnabled)
{
    if(mRemoteMgr->modeMgr()->mode() != FileMode::Simulation || localSessionName.isEmpty())
        newEnabled = false; // Can discover only during simulation

    if(mPeerDiscoveryEnabled == newEnabled)
        return;

    mPeerDiscoveryEnabled = newEnabled;

    if(mPeerDiscoveryEnabled)
    {
        // Start broadcasting
        updateAddresses();

        // Ensure server is running
        mClient->setCommunicationEnabled(true);

        setServerPort(mClient->getServerPort());

        peerBroadcastSocket.bind(QHostAddress::Any, PeerBroadcastPort,
                                 QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

        peerBroadcastTimer.start(BroadcastInterval, this);

        sendPeerBroadcastDatagram();
    }
    else
    {
        // Stop broadcasting
        peerBroadcastTimer.stop();
        peerBroadcastSocket.close();
    }

    emit enabledChanged();

    emit mRemoteMgr->networkStateChanged();
}

bool PeerManager::isTraintasticDiscoveryEnabled() const
{
    return mTraintasticDiscoveryEnabled;
}

void PeerManager::setTraintasticDiscoveryEnabled(bool newEnabled)
{
    if(mRemoteMgr->modeMgr()->mode() != FileMode::Simulation)
        newEnabled = false; // Can discover only during simulation

    if(mTraintasticDiscoveryEnabled == newEnabled)
        return;

    mTraintasticDiscoveryEnabled = newEnabled;

    if(mTraintasticDiscoveryEnabled)
    {
        // Start broadcasting
        updateAddresses();

        // Do not bind UDP socket, system will choose port on first write
        // Otherwise traintastic-simulator fails to bind if started after us
        traintasticBroadcastTimer.start(BroadcastInterval, this);

        qDebug() << "Traintastic discovery ENABLED";
        sendTraintasticBroadcastDatagram();
    }
    else
    {
        // Stop broadcasting
        traintasticBroadcastTimer.stop();
        traintasticBroadcastSocket.close();

        qDebug() << "Traintastic discovery DISABLED";
    }

    emit enabledChanged();

    emit mRemoteMgr->networkStateChanged();
}

ModeManager *PeerManager::modeMgr() const
{
    return mRemoteMgr->modeMgr();
}
