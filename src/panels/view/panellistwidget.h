/**
 * src/panels/view/panellistwidget.h
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

#ifndef PANEL_LISTWIDGET_H
#define PANEL_LISTWIDGET_H

#include <QWidget>

#include "../../enums/filemodes.h"

class PanelListModel;
class PanelScene;

class QLineEdit;
class QPushButton;
class QTableView;

class ViewManager;

class PanelListWidget : public QWidget
{
    Q_OBJECT
public:
    PanelListWidget(ViewManager *mgr, PanelListModel *model, QWidget *parent = nullptr);

    PanelListModel *model() const;

private slots:
    void onFileModeChanged(FileMode mode);

    void addScene();
    void removeCurrentScene();

    void onSceneDoubleClicked(const QModelIndex& idx);

    void showViewContextMenu(const QPoint& pos);

private:
    QTableView *mView;

    QPushButton *addBut;
    QPushButton *remBut;

    ViewManager *mViewMgr;
    PanelListModel *mModel;
};

class PanelSceneOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    PanelSceneOptionsWidget(PanelScene *scene, QWidget *parent = nullptr);

    void setEditingAllowed(bool value);

private slots:
    void setSceneName();
    void onNameTextEdited();
    void setSceneLongName();

private:
    void setNameValid(bool valid);

private:
    PanelScene *mScene = nullptr;

    QLineEdit *mNameEdit = nullptr;
    QLineEdit *mLongNameEdit = nullptr;

    QPalette normalEditPalette;
};

#endif // PANEL_LISTWIDGET_H
