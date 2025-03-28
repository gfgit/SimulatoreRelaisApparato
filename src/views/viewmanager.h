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
class ModeManager;
class UILayoutsModel;

class CircuitWidget;
class CircuitScene;
class AbstractNodeGraphItem;
class CableGraphItem;

class PanelWidget;
class PanelScene;
class AbstractPanelItem;

class AbstractSimulationObject;

namespace KDDockWidgets::QtWidgets
{
class DockWidget;
}

class ViewManager : public QObject
{
    Q_OBJECT
public:
    enum class ViewType
    {
        Circuit = 0,
        Panel = 1
    };

    typedef KDDockWidgets::QtWidgets::DockWidget DockWidget;

    explicit ViewManager(MainWindow *parent);
    ~ViewManager();

    static ViewManager *self();

    // Circuits
    CircuitWidget *activeCircuitView() const;
    CircuitWidget *addCircuitView(CircuitScene *scene,
                                  bool forceNew = false);
    void showCircuitSceneProperties(CircuitScene *scene);

    // Panels
    PanelWidget *activePanelView() const;
    PanelWidget *addPanelView(PanelScene *scene, bool forceNew);
    void showPanelSceneProperties(PanelScene *scene);

    // Objects
    void showObjectProperties(AbstractSimulationObject *item);

    void closeAllEditDocks();
    void closeAllFileSpecificDocks();
    void closeAll();

    ViewType currentViewType() const;

    ModeManager *modeMgr() const;

    AbstractSimulationObject *createNewObjectDlg(const QString &objType,
                                                 QWidget *parent);

    void ensureCircuitItemIsVisible(AbstractNodeGraphItem *item,
                                    bool forceNew, bool adjustZoom);

    void clearLayouts();
    void loadLayoutFile();
    void saveLayoutFile();

    void loadStartLayout();

    inline UILayoutsModel *getLayoutsModel() const
    {
        return mLayoutsModel;
    }

signals:
    void currentViewTypeChanged(ViewType newVal);
    void activeViewChanged();

    void allowNodeBatchEditChanged(bool allow, bool sameType);

public slots:
    void startEditNewCableOnActiveView();
    void addNodeToActiveView(const QString& nodeType);
    void showCircuitListView();
    void showPanelListView();
    void showObjectListView(const QString &objType);

    bool batchCircuitNodeEdit(bool objectReplace);
    bool batchPanelItemEdit(bool objectReplace);

private slots:
    void onCircuitViewDestroyed(QObject *obj);
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);
    void nodeSelectionChanged();

    void panelItemEditRequested(AbstractPanelItem *item);
    void onPanelViewDestroyed(QObject *obj);

    void onFileModeChanged(FileMode mode, FileMode oldMode);

private:
    MainWindow *mainWin() const;

    void setCurrentViewType(ViewType newCurrentViewType);

    friend class CircuitWidget;
    void setActiveCircuit(CircuitWidget *w);
    void updateDockName(CircuitWidget *w);
    int getUniqueNum(CircuitScene *scene, CircuitWidget *self) const;

    friend class PanelWidget;
    void setActivePanel(PanelWidget *w);
    void updateDockName(PanelWidget *w);
    int getUniqueNum(PanelScene *scene, PanelWidget *self) const;

    void setEditDocksEnabled(bool enabled);

private:
    friend class LayoutLoader;
    UILayoutsModel *mLayoutsModel = nullptr;

    // File specific views
    CircuitWidget *mActiveCircuitView = nullptr;
    QHash<CircuitWidget *, DockWidget *> mCircuitViews;

    PanelWidget *mActivePanelView = nullptr;
    QHash<PanelWidget *, DockWidget *> mPanelViews;

    // Edit views
    QHash<CircuitScene *, DockWidget *> mCircuitEdits;
    QHash<PanelScene *, DockWidget *> mPanelEdits;

    QHash<AbstractSimulationObject *, DockWidget *> mObjectEdits;

    // General views
    QHash<QString, DockWidget *> mObjectListDocks;

    QPointer<DockWidget> mCircuitListViewDock;
    QPointer<DockWidget> mPanelListViewDock;

    ViewType mCurrentViewType = ViewType::Circuit;
};

#endif // VIEWMANAGER_H
