// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PEER_SERVER_H
#define PEER_SERVER_H

#include <QTcpServer>

class PeerConnection;

class PeerServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit PeerServer(QObject *parent = nullptr);

    bool isEnabled() const;
    void setEnabled(bool newEnabled);

signals:
    void newConnection(PeerConnection *connection);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    bool mEnabled = false;
};

#endif // PEER_SERVER_H
