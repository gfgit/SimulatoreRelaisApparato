/**
 * src/network/view/replicaslistwidget.h
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

#ifndef REPLICAS_LIST_WIDGET_H
#define REPLICAS_LIST_WIDGET_H

#include <QWidget>

#include "../../enums/filemodes.h"

class QPushButton;
class QTableView;

class ViewManager;
class ReplicasModel;
class RemoteSession;

class ReplicasListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReplicasListWidget(ViewManager *viewMgr,
                                QWidget *parent = nullptr);

    void resizeColumns();

private slots:
    void onFileModeChanged(FileMode mode);

    void addReplica();
    void removeReplica();

    void onRemoteSessionRemoved(RemoteSession *remoteSession);

private:
    ViewManager *mViewMgr;

    QTableView *mView;
    QPushButton *addBut;
    QPushButton *remBut;

    ReplicasModel *mModel;

    RemoteSession *mLastSession = nullptr;
    QString mLastObjType;
    bool mUseRemoteSession = false;
};

#endif // REPLICAS_LIST_WIDGET_H
