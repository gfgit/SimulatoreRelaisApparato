/**
 * src/network/remotesession.h
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

#ifndef REMOTESESSION_H
#define REMOTESESSION_H

#include <QObject>
#include <QHash>

class PeerConnection;

class AbstractSimulationObject;
class RemoteCircuitBridge;

class RemoteManager;

class RemoteSession : public QObject
{
    Q_OBJECT
public:
    struct BridgeResponse
    {
        QVector<quint64> failedIds;
        QHash<quint64, quint64> newMappings;
    };

    struct BridgeListItem
    {
        quint64 peerNodeId;
        QString peerNodeName;
        QString localNodeName;
    };

    explicit RemoteSession(const QString &sessionName, RemoteManager *remoteMgr);
    ~RemoteSession();

    RemoteManager *remoteMgr() const;

    bool setSessionName(const QString& newName);
    inline QString getSessionName() const { return mSessionName; }

    void addRemoteBridge(RemoteCircuitBridge *bridge);
    void removeRemoteBridge(RemoteCircuitBridge *bridge);

    inline RemoteCircuitBridge *getBridgeAt(int bridgeId) const
    {
        return mBridges.value(bridgeId, nullptr);
    }

    inline const QVector<RemoteCircuitBridge *>& getBridges() const
    {
        return mBridges;
    }

    inline PeerConnection *getConnection() const
    {
        return mPeerConn;
    }

    void onConnected(PeerConnection *conn);
    void onDisconnected();

    void sendBridgesStatusToPeer();
    void sendBridgesToPeer();

    bool isRemoteBridgeNameAvailable(const QString& name) const;

public:
    void onRemoteBridgeResponseReceived(PeerConnection *conn,
                                        const BridgeResponse &msg);

    void onRemoteBridgeListReceived(PeerConnection *conn,
                                    const QVector<BridgeListItem> &list);

    void onRemoteBridgeModeChanged(quint64 localNodeId,
                                   qint8 mode, qint8 pole, qint8 replyToMode);

    void onLocalBridgeModeChanged(quint64 peerNodeId,
                                  qint8 mode, qint8 pole, qint8 replyToMode);

private:
    QString mSessionName;
    PeerConnection *mPeerConn = nullptr;

    QVector<RemoteCircuitBridge *> mBridges;

    struct RemoteObject
    {
        AbstractSimulationObject *mLocalObject;
        QString mRemoteCustomName;
    };
};

#endif // REMOTESESSION_H
