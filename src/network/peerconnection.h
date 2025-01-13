/**
 * src/network/peerconnection.h
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

#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QBasicTimer>
#include <QCborStreamReader>
#include <QCborStreamWriter>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QString>
#include <QTcpSocket>
#include <QTimer>

class RemoteManager;

class PeerConnection : public QTcpSocket
{
    Q_OBJECT

public:
    enum ConnectionState {
        WaitingForGreeting,
        ReadingGreeting,
        ProcessingGreeting,
        ReadyForUse
    };
    enum DataType {
        PlainText,
        Ping,
        Pong,
        Greeting,
        BridgeStatus,
        BridgeList,
        BridgeResponse,
        Undefined
    };

    enum class Side
    {
        Server = 0,
        Client = 1
    };

    explicit PeerConnection(QObject *parent = nullptr);
    explicit PeerConnection(qintptr socketDescriptor, QObject *parent = nullptr);
    ~PeerConnection();

    QString sessionName() const;
    QString nickName() const;

    void setGreetingMessage(const QString &message, const QByteArray &uniqueId);
    bool sendMessage(const QString &message);

    QByteArray uniqueId() const;

    Side side() const;
    void setSide(Side newSide);

    inline quint64 getHashedSessionName() const
    {
        return hashedSessionName;
    }

    inline void setRemoteMgr(RemoteManager *mgr)
    {
        remoteMgr = mgr;
    }

    void sendBridgeStatus(quint64 peerNodeId, qint8 mode, qint8 pole);
    void sendCustonMsg(DataType t, const QCborValue& v);

signals:
    void readyForUse();
    void newMessage(const QString &from, const QString &message);

protected:
    void timerEvent(QTimerEvent *timerEvent) override;

private slots:
    void processReadyRead();
    void sendPing();
    void sendGreetingMessage();

private:
    bool hasEnoughData();
    void processGreeting();
    void processData();

    RemoteManager *remoteMgr = nullptr;
    QCborStreamReader reader;
    QCborStreamWriter writer;
    QString greetingMessage = tr("undefined");
    QString peerSessionName = tr("unknown");
    QString peerNickName;
    QTimer pingTimer;
    QElapsedTimer pongTime;
    QString buffer;
    QByteArray byteBuffer;
    QByteArray localUniqueId;
    QByteArray peerUniqueId;
    ConnectionState state = WaitingForGreeting;
    DataType currentDataType = Undefined;
    QBasicTimer transferTimer;
    bool isGreetingMessageSent = false;

    quint64 hashedSessionName = 0;

    Side mSide = Side::Server;
};

#endif
