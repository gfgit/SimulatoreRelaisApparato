/**
 * src/network/replicasmodel.h
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

#ifndef REPLICASMODEL_H
#define REPLICASMODEL_H

#include <QAbstractTableModel>

class ReplicaObjectManager;
class RemoteSession;

class ReplicasModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        ObjectCol = 0,
        TypeCol,
        SessionDeviceCol,
        CustomNameIDCol,
        NCols
    };

    explicit ReplicasModel(ReplicaObjectManager *mgr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    ReplicaObjectManager *replicaMgr() const;

    RemoteSession *getSessionAt(int row) const;
    bool setRemoteSessionAt(int row, RemoteSession *newSession);

    bool removeAt(int row);

private:
    friend class ReplicaObjectManager;

    inline void resetModel(bool phase)
    {
        if(phase)
            beginResetModel();
        else
            endResetModel();
    }

    inline void removeItemAt(int row, bool phase)
    {
        if(phase)
            beginRemoveRows(QModelIndex(), row, row);
        else
            endRemoveRows();
    }

    inline void addItemAt(int row, bool phase)
    {
        if(phase)
            beginInsertRows(QModelIndex(), row, row);
        else
            endInsertRows();
    }
};

#endif // REPLICASMODEL_H
