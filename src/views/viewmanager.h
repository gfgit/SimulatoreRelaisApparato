/**
 * src/views/viewmanager.h
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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QObject>
#include <QPointer>
#include <QHash>

#include "../enums/filemodes.h"

class MainWindow;

class CircuitWidget;

class CircuitScene;
class AbstractNodeGraphItem;
class CableGraphItem;

class GenericLeverObject;

class AbstractRelais;

namespace KDDockWidgets::QtWidgets
{
class DockWidget;
}

class ViewManager : public QObject
{
    Q_OBJECT
public:
    typedef KDDockWidgets::QtWidgets::DockWidget DockWidget;

    explicit ViewManager(MainWindow *parent);
    ~ViewManager();

    CircuitWidget *activeCircuitView() const;

    CircuitWidget *addCircuitView(CircuitScene *scene,
                                  bool forceNew = false);

    void showCircuitSceneEdit(CircuitScene *scene);
    void showLeverEdit(GenericLeverObject *lever);
    void showRelayEdit(AbstractRelais *relay);

    void closeAllEditDocks();
    void closeAllFileSpecificDocks();
    void closeAll();

public slots:
    void startEditNEwCableOnActiveView();
    void addNodeToActiveView(const QString& nodeType);
    void showCircuitListView();
    void showRelayListView();
    void showLeverListView();

private slots:
    void onCircuitViewDestroyed(QObject *obj);

    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

    void onFileModeChanged(FileMode mode, FileMode oldMode);

private:
    friend class CircuitWidget;
    void setActiveCircuit(CircuitWidget *w);

    void updateDockName(CircuitWidget *w);

    MainWindow *mainWin();

    int getUniqueNum(CircuitScene *scene, CircuitWidget *self) const;

private:
    // File specific views
    CircuitWidget *mActiveCircuitView = nullptr;

    QHash<CircuitWidget *, DockWidget *> mCircuitViews;

    // Edit views
    QHash<CircuitScene *, DockWidget *> mCircuitEdits;
    QHash<GenericLeverObject *, DockWidget *> mLeverEdits;
    QHash<AbstractRelais *, DockWidget *> mRelayEdits;

    // General views
    QPointer<DockWidget> mCircuitListViewDock;
    QPointer<DockWidget> mRelaisListViewDock;
    QPointer<DockWidget> mLeverListViewDock;
};

#endif // VIEWMANAGER_H
