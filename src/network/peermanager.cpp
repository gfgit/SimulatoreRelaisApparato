// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2018 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "peerclient.h"
#include "peerconnection.h"
#include "peermanager.h"

#include <QNetworkInterface>
#include <QUuid>

static const qint32 BroadcastInterval = 2000;
static const unsigned broadcastPort = 45000;

static QString getDefaultSessionName()
{
    static const char *envVariables[] = {
        "USERNAME", "USER", "USERDOMAIN", "HOSTNAME", "DOMAINNAME"
    };

    QString sessionName;

    for (const char *varname : envVariables)
    {
        sessionName = qEnvironmentVariable(varname);
        if (!sessionName.isNull())
            break;
    }

    if (sessionName.isEmpty())
        sessionName = "unknown";

    return sessionName;
}

PeerManager::PeerManager(PeerClient *client)
    : QObject(client), client(client)
{
    setSessionName(getDefaultSessionName());

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
    return mSessionName;
}

void PeerManager::setSessionName(const QString &str)
{
    if(client->isCommunicationEnabled())
        return; // Name can be changed only when offline

    QString newName = str.simplified();
    if(mSessionName == newName)
        return;

    mSessionName = newName;

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
        writer.startArray(3);
        writer.append(localUniqueId);
        writer.append(mSessionName);
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

        int senderServerPort;
        QByteArray peerUniqueId;
        QString peerSessionName;
        {
            // decode the datagram
            QCborStreamReader reader(datagram);
            if (reader.lastError() != QCborError::NoError || !reader.isArray())
                continue;
            if (!reader.isLengthKnown() || reader.length() != 3)
                continue;

            reader.enterContainer();
            if (reader.lastError() != QCborError::NoError || !reader.isByteArray())
                continue;
            auto r = reader.readByteArray();
            while (r.status == QCborStreamReader::Ok)
            {
                peerUniqueId = r.data;
                r = reader.readByteArray();
            }

            auto r2 = reader.readString();
            while (r2.status == QCborStreamReader::Ok)
            {
                peerSessionName = r2.data;
                r2 = reader.readString();
            }

            if (reader.lastError() != QCborError::NoError || !reader.isUnsignedInteger())
                continue;
            senderServerPort = reader.toInteger();
        }

        if (peerUniqueId == localUniqueId)
            continue;

        if (!client->hasConnection(peerUniqueId, peerSessionName))
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
    if(mEnabled == newEnabled)
        return;

    mEnabled = newEnabled;

    if(mEnabled)
    {
        // Start broadcasting

        // Ensure server is running
        client->setCommunicationEnabled(true);

        setServerPort(client->getServerPort());

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
}
