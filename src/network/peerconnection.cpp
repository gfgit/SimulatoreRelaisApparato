/**
 * src/network/peerconnection.cpp
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

#include "peerconnection.h"

#include "remotesession.h"

#include <QTimerEvent>

#include <QCborValue>
#include <QCborMap>
#include <QCborArray>

using namespace std::chrono_literals;

static constexpr auto TransferTimeout = 30s;
static constexpr auto PongTimeout = 60s;
static constexpr auto PingInterval = 5s;

/*
 * Protocol is defined as follows, using the CBOR Data Definition Language:
 *
 *  protocol    = [
 *     greeting,        ; must start with a greeting command
 *     * command        ; zero or more regular commands after
 *  ]
 *  command     = plaintext / ping / pong / greeting
 *  plaintext   = { 0 => text }
 *  ping        = { 1 => null }
 *  pong        = { 2 => null }
 *  greeting    = { 3 => { text, bytes } }
 */

PeerConnection::PeerConnection(QObject *parent)
    : QTcpSocket(parent), writer(this)
{
    pingTimer.setInterval(PingInterval);

    connect(this, &QTcpSocket::readyRead, this,
            &PeerConnection::processReadyRead);
    connect(this, &QTcpSocket::disconnected,
            &pingTimer, &QTimer::stop);
    connect(&pingTimer, &QTimer::timeout,
            this, &PeerConnection::sendPing);
    connect(this, &QTcpSocket::connected,
            this, &PeerConnection::onConnected);
}

PeerConnection::PeerConnection(qintptr socketDescriptor, QObject *parent)
    : PeerConnection(parent)
{
    setSocketDescriptor(socketDescriptor);

    // Server side
    setSocketOption(QTcpSocket::LowDelayOption, 1);

    reader.setDevice(this);
}

PeerConnection::~PeerConnection()
{
    if (isGreetingMessageSent && QAbstractSocket::state() != QAbstractSocket::UnconnectedState) {
        // Indicate clean shutdown.
        writer.endArray();
        waitForBytesWritten(2000);
    }
}

QString PeerConnection::sessionName() const
{
    return peerSessionName;
}

QString PeerConnection::nickName() const
{
    return peerNickName;
}

void PeerConnection::setGreetingMessage(const QString &message, const QByteArray &uniqueId)
{
    greetingMessage = message;
    localUniqueId = uniqueId;
}

QByteArray PeerConnection::uniqueId() const
{
    return peerUniqueId;
}

bool PeerConnection::sendMessage(const QString &message)
{
    if (message.isEmpty())
        return false;

    writer.startMap(1);
    writer.append(PlainText);
    writer.append(message);
    writer.endMap();
    return true;
}

void PeerConnection::sendBridgeStatus(quint64 peerNodeId, qint8 mode, qint8 pole, qint8 replyToMode)
{
    const quint64 arr[2] = {quint64(peerNodeId), quint64(mode | (pole << 8) | (replyToMode << 16))};

    writer.startMap(1);
    writer.append(BridgeStatus);
    writer.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&arr),
                                          2 * sizeof(quint64)));
    writer.endMap();
}

void PeerConnection::sendCustonMsg(DataType t, const QCborValue &v)
{
    writer.startMap(1);
    writer.append(t);
    v.toCbor(writer);
    writer.endMap();
}

void PeerConnection::closeConnection()
{
    if(remoteSession)
        remoteSession->onDisconnected();

    if(state() != UnconnectedState && state() != ClosingState)
        disconnectFromHost();
}

void PeerConnection::timerEvent(QTimerEvent *timerEvent)
{
    if (timerEvent->timerId() == transferTimer.timerId())
    {
        abort();
        transferTimer.stop();
    }
}

void PeerConnection::onConnected()
{
    // Client side
    setSocketOption(QTcpSocket::LowDelayOption, 1);

    sendGreetingMessage();
}

void PeerConnection::processReadyRead()
{
    // we've got more data, let's parse
    reader.reparse();
    while (reader.lastError() == QCborError::NoError)
    {
        if (mState == WaitingForGreeting)
        {
            if (!reader.isArray())
                break;                  // protocol error

            reader.enterContainer();    // we'll be in this array forever
            mState = ReadingGreeting;
        }
        else if (reader.containerDepth() == 1)
        {
            // Current state: no command read
            // Next state: read command ID
            if (!reader.hasNext())
            {
                reader.leaveContainer();
                disconnectFromHost();
                return;
            }

            if (!reader.isMap() || !reader.isLengthKnown() || reader.length() != 1)
                break;                  // protocol error
            reader.enterContainer();
        }
        else if (currentDataType == Undefined)
        {
            // Current state: read command ID
            // Next state: read command payload
            if (!reader.isInteger())
                break;                  // protocol error
            currentDataType = DataType(reader.toInteger());
            reader.next();
        }
        else
        {
            // Current state: read command payload
            if (currentDataType == Greeting)
            {
                if (mState == ReadingGreeting)
                {
                    if (!reader.isContainer() || !reader.isLengthKnown() || reader.length() != 2)
                        break; // protocol error
                    mState = ProcessingGreeting;
                    reader.enterContainer();
                }

                if (mState != ProcessingGreeting)
                    break; // protocol error

                if (reader.isString())
                {
                    auto r = reader.readString();
                    buffer += r.data;
                }
                else if (reader.isByteArray())
                {
                    auto r = reader.readByteArray();
                    peerUniqueId += r.data;
                    if (r.status == QCborStreamReader::EndOfString)
                    {
                        reader.leaveContainer();
                        processGreeting();
                    }
                }
                if (mState == ProcessingGreeting)
                    continue;
            }
            else if (currentDataType == BridgeList)
            {
                if(!reader.isMap())
                    break; // protocol error

                QVector<RemoteSession::BridgeListItem> list;
                if(reader.isLengthKnown())
                    list.reserve(reader.length());

                reader.enterContainer();
                while (reader.lastError() == QCborError::NoError && reader.hasNext())
                {
                    RemoteSession::BridgeListItem item;
                    item.peerNodeId = reader.toUnsignedInteger();
                    reader.next();

                    reader.enterContainer();
                    auto strResult = reader.readString();
                    while (strResult.status == QCborStreamReader::Ok)
                    {
                        item.peerNodeName = strResult.data;
                        strResult = reader.readString();
                    }

                    strResult = reader.readString();
                    while (strResult.status == QCborStreamReader::Ok)
                    {
                        item.localNodeName = strResult.data;
                        strResult = reader.readString();
                    }
                    reader.leaveContainer();

                    list.append(item);
                }

                if (reader.lastError() == QCborError::NoError)
                {
                    reader.leaveContainer();

                    if(remoteSession)
                    {
                        remoteSession->onRemoteBridgeListReceived(list);
                    }
                }
            }
            else if (currentDataType == BridgeResponse)
            {
                if(!reader.isArray())
                    break; // protocol error

                reader.enterContainer();

                RemoteSession::BridgeResponse msg;

                {
                    if(!reader.isArray())
                        break;

                    if(reader.isLengthKnown())
                        msg.failedIds.reserve(reader.length());

                    reader.enterContainer();

                    while (reader.lastError() == QCborError::NoError && reader.hasNext())
                    {
                        msg.failedIds.append(reader.toUnsignedInteger());
                        reader.next();
                    }

                    reader.leaveContainer();
                }

                if (reader.lastError() == QCborError::NoError)
                {
                    reader.leaveContainer();

                    if(remoteSession)
                    {
                        remoteSession->onRemoteBridgeResponseReceived(msg);
                    }
                }
            }
            else if (currentDataType == ReplicaList)
            {
                if(!reader.isArray())
                    break; // protocol error

                const QCborValue msg = QCborValue::fromCbor(reader);

                if (reader.lastError() == QCborError::NoError)
                {
                    if(remoteSession && msg.isArray())
                    {
                        remoteSession->onReplicaListReceived(msg.toArray());
                    }
                }
            }
            else if (currentDataType == ReplicaResponse)
            {
                if(!reader.isArray())
                    break; // protocol error

                const QCborValue msg = QCborValue::fromCbor(reader);

                if (reader.lastError() == QCborError::NoError)
                {
                    if(remoteSession && msg.isArray())
                    {
                        remoteSession->onReplicaResponseReceived(msg.toArray());
                    }
                }
            }
            else if (currentDataType == ReplicaStatus)
            {
                if(!reader.isArray())
                    break; // protocol error

                reader.enterContainer();
                const quint64 replicaId = reader.toUnsignedInteger();
                reader.next();
                const QCborValue objState = QCborValue::fromCbor(reader);

                if (reader.lastError() == QCborError::NoError)
                {
                    reader.leaveContainer();

                    if(remoteSession && objState.isMap())
                    {
                        remoteSession->onSourceObjectStateReceived(replicaId, objState.toMap());
                    }
                }
            }
            else if (reader.isString())
            {
                auto r = reader.readString();
                buffer += r.data;
                if (r.status != QCborStreamReader::EndOfString)
                    continue;
            }
            else if (reader.isByteArray())
            {
                auto r = reader.readByteArray();
                byteBuffer += r.data;
                if (r.status != QCborStreamReader::EndOfString)
                    continue;
            }
            else if (reader.isNull())
            {
                reader.next();
            }
            else
            {
                break; // protocol error
            }

            // Next state: no command read
            reader.leaveContainer();
            transferTimer.stop();

            processData();
        }
    }

    if (reader.lastError() != QCborError::EndOfFile)
        abort();       // parse error

    if (transferTimer.isActive() && reader.containerDepth() > 1)
        transferTimer.start(TransferTimeout, this);
}

void PeerConnection::sendPing()
{
    if (pongTime.durationElapsed() > PongTimeout)
    {
        abort();
        return;
    }

    writer.startMap(1);
    writer.append(Ping);
    writer.append(nullptr);     // no payload
    writer.endMap();
}

void PeerConnection::sendGreetingMessage()
{
    writer.startArray();        // this array never ends

    writer.startMap(1);
    writer.append(Greeting);
    writer.startArray(2);
    writer.append(greetingMessage);
    writer.append(localUniqueId);
    writer.endArray();
    writer.endMap();
    isGreetingMessageSent = true;

    if (!reader.device())
        reader.setDevice(this);
}

void PeerConnection::processGreeting()
{
    peerSessionName = buffer;
    peerNickName = peerSessionName + '@' + peerAddress().toString() + ':'
            + QString::number(peerPort());
    currentDataType = Undefined;
    buffer.clear();

    if (!isValid())
    {
        abort();
        return;
    }

    if (!isGreetingMessageSent)
        sendGreetingMessage();

    pingTimer.start();
    pongTime.start();
    mState = ReadyForUse;
    emit readyForUse();
}

void PeerConnection::processData()
{
    switch (currentDataType)
    {
    case PlainText:
        emit newMessage(peerNickName, buffer);
        break;
    case BridgeStatus:
    {
        if(remoteSession && size_t(byteBuffer.size()) >= 2 * sizeof(quint64))
        {
            const quint64 *arr = reinterpret_cast<const quint64 *>(byteBuffer.constData());
            quint64 localNodeId = arr[0];
            qint8 mode = qint8(arr[1] & 0xFF);
            qint8 pole = qint8((arr[1] >> 8) & 0xFF);
            qint8 replyToMode = qint8((arr[1] >> 16) & 0xFF);
            remoteSession->onRemoteBridgeModeChanged(localNodeId,
                                                     mode, pole, replyToMode);
        }
        break;
    }
    case Ping:
        writer.startMap(1);
        writer.append(Pong);
        writer.append(nullptr);     // no payload
        writer.endMap();
        break;
    case Pong:
        pongTime.restart();
        break;
    default:
        break;
    }

    currentDataType = Undefined;
    byteBuffer.clear();
    buffer.clear();
}

PeerConnection::Side PeerConnection::side() const
{
    return mSide;
}

void PeerConnection::setSide(Side newSide)
{
    mSide = newSide;
}
