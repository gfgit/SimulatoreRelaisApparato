/**
 * src/objects/lever/view/genericleverlistwidget.h
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
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

#ifndef GENERIC_LEVER_LISTWIDGET_H
#define GENERIC_LEVER_LISTWIDGET_H

#include <QWidget>

#include "../../../enums/filemodes.h"

class GenericLeverModel;
class QSortFilterProxyModel;

class QPushButton;
class QTableView;

class ViewManager;

class GenericLeverListWidget : public QWidget
{
    Q_OBJECT
public:
    GenericLeverListWidget(ViewManager *mgr, GenericLeverModel *model, QWidget *parent = nullptr);

    GenericLeverModel *model() const;

private slots:
    void onFileModeChanged(FileMode mode);

    void addLever();
    void removeCurrentLever();
private:
    QTableView *mView;

    QPushButton *addBut;
    QPushButton *remBut;

    ViewManager *mViewMgr;
    GenericLeverModel *mModel;

    QSortFilterProxyModel *mProxyModel;
};

#endif // GENERIC_LEVER_LISTWIDGET_H
