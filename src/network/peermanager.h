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
class PeerClient;
class PeerConnection;

class PeerManager : public QObject
{
    Q_OBJECT

public:
    explicit PeerManager(PeerClient *client, ModeManager *mgr);

    void setServerPort(int port);

    QString sessionName() const;
    void setSessionName(const QString& str);

    QByteArray uniqueId() const;

    void startBroadcasting();
    void stopBroadcasting();

    bool isLocalHostAddress(const QHostAddress &address) const;

    bool isDiscoveryEnabled() const;
    void setDiscoveryEnabled(bool newEnabled);

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

signals:
    void sessionNameChanged(const QString& newName);

    void newConnection(PeerConnection *connection);

    void enabledChanged();

private slots:
    void sendBroadcastDatagram();
    void readBroadcastDatagram();

private:
    void updateAddresses();

private:
    ModeManager *mModeMgr = nullptr;
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
