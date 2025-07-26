/**
 * src/network/remotesessionsmodel.cpp
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

#include "remotesessionsmodel.h"

#include "remotemanager.h"
#include "remotesession.h"

#include "../views/modemanager.h"

#include <QColor>

RemoteSessionsModel::RemoteSessionsModel(RemoteManager *mgr)
    : QAbstractTableModel(mgr)
{
}

RemoteManager *RemoteSessionsModel::remoteMgr() const
{
    return static_cast<RemoteManager *>(parent());
}

QVariant RemoteSessionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case NameCol:
            return tr("Name");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int RemoteSessionsModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mRemoteSessions.count();
}

int RemoteSessionsModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant RemoteSessionsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mRemoteSessions.size())
        return QVariant();

    const RemoteSession *remoteSession = mRemoteSessions.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case NameCol:
            return remoteSession->getSessionName();
        default:
            break;
        }

        break;
    }
    case Qt::DecorationRole:
    {
        switch (idx.column())
        {
        case NameCol:
        {
            // Show connection status
            QColor statusColor = Qt::black;
            if(remoteSession->getConnection())
                statusColor = Qt::green;
            return statusColor;
        }
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

bool RemoteSessionsModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(remoteMgr()->modeMgr()->mode() != FileMode::Editing)
        return false;

    if (!idx.isValid() || idx.row() >= mRemoteSessions.size())
        return false;

    RemoteSession *remoteSession = mRemoteSessions.at(idx.row());

    if(role == Qt::EditRole)
    {
        switch (idx.column())
        {
        case NameCol:
            return remoteSession->setSessionName(value.toString());
        default:
            break;
        }
    }

    return false;
}

Qt::ItemFlags RemoteSessionsModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mRemoteSessions.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(remoteMgr()->modeMgr()->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

void RemoteSessionsModel::updateSessionStatus()
{
    emit dataChanged(index(0, NameCol),
                     index(rowCount() - 1, NameCol));
}

void RemoteSessionsModel::sortItems()
{
    beginResetModel();

    std::sort(mRemoteSessions.begin(),
              mRemoteSessions.end(),
              [](RemoteSession *a, RemoteSession *b) -> bool
    {
        return a->getSessionName() < b->getSessionName();
    });

    endResetModel();
}

void RemoteSessionsModel::clear()
{
    beginResetModel();
    mRemoteSessions.clear();
    endResetModel();
}

void RemoteSessionsModel::addRemoteSession(RemoteSession *remoteSession)
{
    beginResetModel();

    mRemoteSessions.append(remoteSession);

    std::sort(mRemoteSessions.begin(),
              mRemoteSessions.end(),
              [](RemoteSession *a, RemoteSession *b) -> bool
    {
        return a->getSessionName() < b->getSessionName();
    });

    endResetModel();
}

void RemoteSessionsModel::removeRemoteSession(RemoteSession *remoteSession)
{
    const int row = mRemoteSessions.indexOf(remoteSession);
    if(row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mRemoteSessions.removeAt(row);
    endRemoveRows();
}
