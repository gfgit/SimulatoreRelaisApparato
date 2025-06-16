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

class RemoteSession;
class RemoteSessionsModel;

class ReplicaObjectManager;

class QJsonObject;

class RemoteManager : public QObject
{
    Q_OBJECT
public:
    explicit RemoteManager(ModeManager *mgr);
    ~RemoteManager();

    ModeManager *modeMgr() const;

    QString sessionName() const;
    void setSessionName(const QString &newSessionName);

    void clear();

    void setOnline(bool val);
    bool isOnline() const;

    void setDiscoveryEnabled(bool val);
    bool isDiscoveryEnabled() const;

    void refreshNetworkAddresses();

    inline bool isSessionReferenced(const QString& name) const
    {
        return mRemoteSessions.contains(name);
    }

    RemoteSession* addRemoteSession(const QString& sessionName);
    RemoteSession* getRemoteSession(const QString& sessionName) const;
    void removeRemoteSession(RemoteSession *remoteSession);

    RemoteSessionsModel *remoteSessionsModel() const;

    ReplicaObjectManager *replicaMgr() const;

    bool loadFromJSON(const QJsonObject& obj);
    void saveToJSON(QJsonObject& obj);

    bool onlineByDefault() const;
    void setOnlineByDefault(bool newOnlineByDefault);

signals:
    void networkStateChanged();
    void remoteSessionRemoved(RemoteSession *s);

private:
    friend class PeerClient;
    void addConnection(PeerConnection *conn);

    friend class RemoteSession;
    bool renameRemoteSession(const QString &fromName, const QString &toName);

private:
    PeerClient *mPeerClient = nullptr;
    PeerManager *mPeerManager = nullptr;
    RemoteSessionsModel *mRemoteSessionsModel = nullptr;
    ReplicaObjectManager *mReplicaMgr = nullptr;
    bool mOnlineByDefault = false;

    QHash<QString, RemoteSession *> mRemoteSessions;
};

#endif // REMOTEMANAGER_H
