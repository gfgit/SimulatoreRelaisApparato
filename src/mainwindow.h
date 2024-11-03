/**
 * src/mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kddockwidgets-qt6/kddockwidgets/MainWindow.h>

class CircuitScene;
class AbstractNodeGraphItem;
class CableGraphItem;
class NodeEditFactory;

class ModeManager;
class ViewManager;

class QListView;

class MainWindow : public KDDockWidgets::QtWidgets::MainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &uniqueName_, QWidget *parent = nullptr);
    ~MainWindow();

    ModeManager *modeMgr() const;

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    void buildMenuBarAndToolBar();
    void updateRecentFileActions();
    void addFileToRecents(const QString& fileName);

    void loadFile(const QString &fileName);

    void locateAppSettings();

    bool maybeSave();

    bool saveFile(const QString &fileName);

private slots:
    void onNew();
    void onOpen();
    void onOpenRecent();
    void onSave();
    void onSaveAs();

    void updateWindowModified();

private:
    // Actions
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionOpen_Recent;
    QAction *actionNew;
    QAction *actionSave_As;

    // Models
    ModeManager *mModeMgr = nullptr;

    // Views
    ViewManager *mViewMgr = nullptr;

    // Relays
    QListView *mRelaisView = nullptr;

    enum
    {
        MaxRecentFiles = 10
    };
    QAction *recentFileActs[MaxRecentFiles] = {};
    QString settingsFile;
};
#endif // MAINWINDOW_H
