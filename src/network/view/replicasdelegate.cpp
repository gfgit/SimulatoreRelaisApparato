/**
 * src/network/view/replicasdelegate.cpp
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

#include "replicasdelegate.h"

#include "../replicasmodel.h"
#include "../remotesessionsmodel.h"

#include <QComboBox>

ReplicasDelegate::ReplicasDelegate(RemoteSessionsModel *sessionsModel, QObject *parent)
    : QStyledItemDelegate{parent}
    , mSessionsModel(sessionsModel)
{
}

QWidget *ReplicasDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &options,
                                        const QModelIndex &index) const
{
    if(index.column() != ReplicasModel::SessionDeviceCol)
        return QStyledItemDelegate::createEditor(parent, options, index);

    QComboBox *combo = new QComboBox(parent);
    combo->setModel(mSessionsModel);

    return combo;
}

void ReplicasDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(index.column() != ReplicasModel::SessionDeviceCol)
    {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    QComboBox *combo = static_cast<QComboBox *>(editor);

    const ReplicasModel *repModel = static_cast<const ReplicasModel *>(index.model());
    RemoteSession *remoteSession = repModel->getSessionAt(index.row());

    const int sessionRow = mSessionsModel->rowForRemoteSession(remoteSession);
    combo->setCurrentIndex(sessionRow);
}

void ReplicasDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column() != ReplicasModel::SessionDeviceCol)
    {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    QComboBox *combo = static_cast<QComboBox *>(editor);
    RemoteSession *remoteSession = mSessionsModel->getRemoteSessionAt(combo->currentIndex());

    ReplicasModel *repModel = static_cast<ReplicasModel *>(model);
    repModel->setRemoteSessionAt(index.row(), remoteSession);
}

// void ReplicasDelegate::onItemClicked()
// {
//     QComboBox *combo = qobject_cast<QComboBox *>(sender());
//     if (combo)
//     {
//         emit commitData(combo);
//         emit closeEditor(combo);
//     }
// }
