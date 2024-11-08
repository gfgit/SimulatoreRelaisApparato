/**
 * src/objects/relais/view/relaislistwidget.h
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

#ifndef RELAISLISTWIDGET_H
#define RELAISLISTWIDGET_H

#include <QWidget>

#include "../../../enums/filemodes.h"

class RelaisModel;
class QSortFilterProxyModel;

class QPushButton;
class QTableView;

class ViewManager;

class RelaisListWidget : public QWidget
{
    Q_OBJECT
public:
    RelaisListWidget(ViewManager *mgr, RelaisModel *model, QWidget *parent = nullptr);

    RelaisModel *model() const;

private slots:
    void onFileModeChanged(FileMode mode);

    void addRelay();
    void removeCurrentRelay();
    void showViewContextMenu(const QPoint &pos);

private:
    QTableView *mView;

    QPushButton *addBut;
    QPushButton *remBut;

    ViewManager *mViewMgr;
    RelaisModel *mModel;

    QSortFilterProxyModel *mProxyModel;
};

#endif // RELAISLISTWIDGET_H
