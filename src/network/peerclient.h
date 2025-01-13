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
