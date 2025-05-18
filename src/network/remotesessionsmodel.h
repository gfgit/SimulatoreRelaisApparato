/**
 * src/network/remotesessionsmodel.h
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

#ifndef REMOTESESSIONSMODEL_H
#define REMOTESESSIONSMODEL_H

#include <QAbstractTableModel>

class RemoteManager;
class RemoteSession;

class RemoteSessionsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        NameCol = 0,
        NCols
    };

    explicit RemoteSessionsModel(RemoteManager *mgr = nullptr);

    RemoteManager *remoteMgr() const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    inline RemoteSession *getRemoteSessionAt(int row) const
    {
        return mRemoteSessions.value(row, nullptr);
    }

    inline int rowForRemoteSession(RemoteSession *remoteSession) const
    {
        return mRemoteSessions.indexOf(remoteSession);
    }

private:
    friend class RemoteManager;
    void sortItems();
    void clear();

    void addRemoteSession(RemoteSession *remoteSession);
    void removeRemoteSession(RemoteSession *remoteSession);

private:
    QVector<RemoteSession *> mRemoteSessions;
};

#endif // REMOTESESSIONSMODEL_H
