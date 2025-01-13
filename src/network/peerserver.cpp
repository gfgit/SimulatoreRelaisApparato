// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "peerconnection.h"
#include "peerserver.h"

PeerServer::PeerServer(QObject *parent)
    : QTcpServer(parent)
{

}

void PeerServer::incomingConnection(qintptr socketDescriptor)
{
    if(!mEnabled)
    {
        qDebug() << "CLOSING CONNECTION - NOT ENABLED!!!";

        // TODO: find better way
        QTcpSocket sock;
        sock.setSocketDescriptor(socketDescriptor);
        sock.close();
        return;
    }

    PeerConnection *connection = new PeerConnection(socketDescriptor, this);
    connection->setSide(PeerConnection::Side::Server);
    emit newConnection(connection);
}

bool PeerServer::isEnabled() const
{
    return mEnabled;
}

void PeerServer::setEnabled(bool newEnabled)
{
    if(mEnabled == newEnabled)
        return;

    mEnabled = newEnabled;

    if(mEnabled)
        listen(QHostAddress::Any);
    else
        close();
}
