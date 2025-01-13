/**
 * src/network/peerserver.cpp
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
