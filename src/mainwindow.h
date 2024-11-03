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
class RelaisModel;
class NodeEditFactory;

class ZoomGraphView;
class QDoubleSpinBox;
class DoubleClickSlider;

class QListView;

class MainWindow : public KDDockWidgets::QtWidgets::MainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &uniqueName_, QWidget *parent = nullptr);
    ~MainWindow();

    CircuitScene *scene() const;

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    void buildToolBar();
    void updateRecentFileActions();
    void addFileToRecents(const QString& fileName);

    void loadFile(const QString &fileName);

    void locateAppSettings();

    bool hasUnsavedChanges() const;
    bool maybeSave();

    bool saveFile(const QString &fileName);

private slots:
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

    void onNew();
    void onOpen();
    void onOpenRecent();
    void onSave();
    void onSaveAs();

    void updateWindowModified();

    void onZoomChanged(double val);
    void onZoomSliderChanged(int val);
    void onZoomSpinChanged(double val);
    void resetZoom();

private:
    // Actions
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionOpen_Recent;
    QAction *actionNew;
    QAction *actionSave_As;

    // Circuits
    CircuitScene *mScene;

    // Relays
    QListView *mRelaisView;
    RelaisModel *mRelaisModel;
    NodeEditFactory *mEditFactory;

    enum
    {
        MaxRecentFiles = 10
    };
    QAction *recentFileActs[MaxRecentFiles];
    QString settingsFile;

    ZoomGraphView *mCircuitView;
    DoubleClickSlider *mZoomSlider;
    QDoubleSpinBox *mZoomSpin;

    // Prevent showing modified file while loading
    bool mIsLoading = false;
};
#endif // MAINWINDOW_H
