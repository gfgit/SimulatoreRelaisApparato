/**
 * src/network/replicasmodel.cpp
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

#include "replicasmodel.h"

#include "replicaobjectmanager.h"
#include "remotesession.h"

#include "remotemanager.h"
#include "../views/modemanager.h"

#include "../objects/abstractsimulationobject.h"

ReplicasModel::ReplicasModel(ReplicaObjectManager *mgr)
    : QAbstractTableModel(mgr)
{

}

QVariant ReplicasModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case ObjectCol:
            return tr("Object");
        case TypeCol:
            return tr("Type");
        case SessionDeviceCol:
        {
            return tr("Session");
        }
        case CustomNameIDCol:
            return tr("Custom Name");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int ReplicasModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : replicaMgr()->mReplicas.size();
}

int ReplicasModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant ReplicasModel::data(const QModelIndex &idx, int role) const
{
    const auto& replicas = replicaMgr()->mReplicas;

    if (!idx.isValid() || idx.row() >= replicas.size() || idx.column() >= NCols)
        return QVariant();

    const ReplicaObjectManager::ReplicaObjectData& repData = replicas.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case ObjectCol:
            return repData.replicaObj->name();
        case TypeCol:
            return tr("Remote"); // TODO: Serial
        case SessionDeviceCol:
        {
            if(repData.remoteSession)
                return repData.remoteSession->getSessionName();
            break;
        }
        case CustomNameIDCol:
            return repData.customName;
        default:
            break;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case CustomNameIDCol:
            return repData.customName;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return QVariant();
}

bool ReplicasModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    ReplicaObjectManager *mgr = replicaMgr();

    if (!idx.isValid() || idx.row() >= mgr->mReplicas.size() || idx.column() >= NCols)
        return false;

    const ReplicaObjectManager::ReplicaObjectData repDataCopy = mgr->mReplicas.at(idx.row());

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case CustomNameIDCol:
        {
            const QString newName = value.toString();
            return mgr->setReplicaObjectSession(repDataCopy.replicaObj,
                                                repDataCopy.remoteSession,
                                                newName);
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return false;
}

Qt::ItemFlags ReplicasModel::flags(const QModelIndex &idx) const
{
    ReplicaObjectManager *mgr = replicaMgr();

    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mgr->mReplicas.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(idx.column() != ObjectCol &&
            mgr->remoteMgr()->modeMgr()->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

ReplicaObjectManager* ReplicasModel::replicaMgr() const
{
    return static_cast<ReplicaObjectManager *>(parent());
}

RemoteSession *ReplicasModel::getSessionAt(int row) const
{
    const auto& replicas = replicaMgr()->mReplicas;
    if (row < 0 || row >= replicas.size())
        return nullptr;

    const ReplicaObjectManager::ReplicaObjectData& repData = replicas.at(row);
    return repData.remoteSession;
}

bool ReplicasModel::setRemoteSessionAt(int row, RemoteSession *newSession)
{
    ReplicaObjectManager *mgr = replicaMgr();
    if (row < 0 || row >= mgr->mReplicas.size())
        return false;

    const ReplicaObjectManager::ReplicaObjectData repDataCopy = mgr->mReplicas.at(row);
    return mgr->setReplicaObjectSession(repDataCopy.replicaObj,
                                        newSession,
                                        repDataCopy.customName);
}

bool ReplicasModel::removeAt(int row)
{
    ReplicaObjectManager *mgr = replicaMgr();
    if (row < 0 || row >= mgr->mReplicas.size())
        return false;

    const ReplicaObjectManager::ReplicaObjectData repDataCopy = mgr->mReplicas.at(row);
    return mgr->removeReplicaObject(repDataCopy.replicaObj);
}
