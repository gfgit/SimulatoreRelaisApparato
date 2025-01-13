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

    QCborStreamReader reader;
    QCborStreamWriter writer;
    QString greetingMessage = tr("undefined");
    QString peerSessionName = tr("unknown");
    QString peerNickName;
    QTimer pingTimer;
    QElapsedTimer pongTime;
    QString buffer;
    QByteArray localUniqueId;
    QByteArray peerUniqueId;
    ConnectionState state = WaitingForGreeting;
    DataType currentDataType = Undefined;
    QBasicTimer transferTimer;
    bool isGreetingMessageSent = false;

    Side mSide = Side::Server;
};

#endif
