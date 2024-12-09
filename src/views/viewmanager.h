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

class AbstractSimulationObject;

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
    void showObjectEdit(AbstractSimulationObject *item);

    void closeAllEditDocks();
    void closeAllFileSpecificDocks();
    void closeAll();

public slots:
    void startEditNEwCableOnActiveView();
    void addNodeToActiveView(const QString& nodeType);
    void showCircuitListView();
    void showObjectListView(const QString &objType);

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

    QHash<AbstractSimulationObject *, DockWidget *> mObjectEdits;

    // General views
    QHash<QString, DockWidget *> mObjectListDocks;

    QPointer<DockWidget> mCircuitListViewDock;
};

#endif // VIEWMANAGER_H
