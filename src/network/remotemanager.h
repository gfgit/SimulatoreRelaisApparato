/**
 * src/network/remotemanager.h
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

#ifndef REMOTEMANAGER_H
#define REMOTEMANAGER_H

#include <QObject>

#include <QHash>

class ModeManager;

class PeerClient;
class PeerManager;
class PeerConnection;

class RemoteCircuitBridge;

class RemoteManager : public QObject
{
    Q_OBJECT
public:
    explicit RemoteManager(ModeManager *mgr);
    ~RemoteManager();

    ModeManager *modeMgr() const;

    QString sessionName() const;
    void setSessionName(const QString &newSessionName);

    void setOnline(bool val);
    bool isOnline() const;

    void setDiscoveryEnabled(bool val);
    bool isDiscoveryEnabled() const;

    bool renameRemoteSession(const QString &fromName, const QString &toName);

    void addRemoteBridge(RemoteCircuitBridge *bridge, const QString& peerSession);
    void removeRemoteBridge(RemoteCircuitBridge *bridge, const QString& peerSession);

    void onLocalBridgeModeChanged(quint64 peerSessionId, quint64 peerNodeId,
                                  qint8 mode, qint8 pole);

    void onRemoteBridgeModeChanged(quint64 peerSessionId, quint64 localNodeId,
                                   qint8 mode, qint8 pole);

    inline bool isSessionReferenced(const QString& name) const
    {
        return mRemoteBridges.contains(name);
    }

    struct BridgeListItem
    {
        quint64 peerNodeId;
        QString peerNodeName;
        QString localNodeName;
    };

    void onRemoteBridgeListReceived(PeerConnection *conn, const QVector<BridgeListItem>& list);

    struct BridgeResponse
    {
        QVector<quint64> failedIds;
        QHash<quint64, quint64> newMappings;
    };
    void onRemoteBridgeResponseReceived(PeerConnection *conn, const BridgeResponse& msg);

signals:
    void networkStateChanged();

private:
    friend class PeerClient;
    void addConnection(PeerConnection *conn);
    void removeConnection(PeerConnection *conn);

    void sendBridgesTo(PeerConnection *conn);
    void sendBridgesStatusTo(PeerConnection *conn);

private:
    PeerClient *mPeerClient = nullptr;
    PeerManager *mPeerManager = nullptr;
    QHash<QString, QVector<RemoteCircuitBridge *>> mRemoteBridges;

    QHash<quint64, PeerConnection *> mConnections;
};

#endif // REMOTEMANAGER_H
