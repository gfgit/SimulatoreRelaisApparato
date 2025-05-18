/**
 * src/network/view/remotesessionlistwidget.h
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

#ifndef REMOTESESSIONLISTWIDGET_H
#define REMOTESESSIONLISTWIDGET_H

#include <QWidget>

#include "../../enums/filemodes.h"

class QPushButton;
class QTableView;

class ViewManager;
class RemoteSessionsModel;

class RemoteSessionListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RemoteSessionListWidget(ViewManager *viewMgr,
                                     QWidget *parent = nullptr);

private slots:
    void onFileModeChanged(FileMode mode);

    void addRemoteSession();
    void removeRemoteSession();

private:
    ViewManager *mViewMgr;

    QTableView *mView;
    QPushButton *addBut;
    QPushButton *remBut;

    RemoteSessionsModel *mModel;
};

#endif // REMOTESESSIONLISTWIDGET_H
