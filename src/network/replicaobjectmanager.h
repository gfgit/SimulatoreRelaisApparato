/**
 * src/network/replicaobjectmanager.h
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

#ifndef REPLICAOBJECTMANAGER_H
#define REPLICAOBJECTMANAGER_H

#include <QObject>
#include <QHash>

class RemoteManager;
class RemoteSession;

class AbstractSimulationObject;
class ReplicasModel;

class QJsonObject;

class ReplicaObjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ReplicaObjectManager(RemoteManager *mgr);
    ~ReplicaObjectManager();

    RemoteManager *remoteMgr() const;

    bool addReplicaObject(AbstractSimulationObject *replicaObj);
    bool removeReplicaObject(AbstractSimulationObject *replicaObj);
    bool setReplicaObjectSession(AbstractSimulationObject *replicaObj,
                                 RemoteSession *remoteSession, const QString& customName);

    bool loadFromJSON(const QJsonObject& obj);
    void saveToJSON(QJsonObject& obj);
    void clear();

    ReplicasModel *replicasModel() const;

private slots:
    void onSourceObjStateChanged(AbstractSimulationObject *obj);
    void onReplicaDestroyed(QObject *obj);
    void onReplicaNameChanged(AbstractSimulationObject *replicaObj,
                              const QString &newName, const QString &oldName);

private:
    friend class RemoteSession;
    void addSourceObject(AbstractSimulationObject *obj,
                         RemoteSession *remoteSession,
                         quint64 replicaId);
    void removeSourceObjects(RemoteSession *remoteSession);

private:
    friend class ReplicasModel;
    ReplicasModel *mReplicasModel = nullptr;

    struct RemoteSessionData
    {
        RemoteSession *remoteSession = nullptr;
        quint64 replicaId = 0;
    };

    struct SourceObjectData
    {
        QList<RemoteSessionData> sessions;
    };

    QHash<AbstractSimulationObject *, SourceObjectData> mSourceObjects;

    struct ReplicaObjectData
    {
        AbstractSimulationObject *replicaObj = nullptr;
        RemoteSession *remoteSession = nullptr;
        QString customName;
    };

    QVector<ReplicaObjectData> mReplicas;
};

#endif // REPLICAOBJECTMANAGER_H
