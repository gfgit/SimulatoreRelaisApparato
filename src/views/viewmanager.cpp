/**
 * src/views/viewmanager.cpp
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

#include "viewmanager.h"

#include "../mainwindow.h"
#include "modemanager.h"

#include "../circuits/edit/nodeeditfactory.h"

#include "../circuits/view/circuitwidget.h"
#include "../circuits/view/circuitlistwidget.h"

#include "../circuits/view/circuitlistmodel.h"
#include "../circuits/circuitscene.h"

#include "../objects/simulationobjectfactory.h"
#include "../objects/abstractsimulationobject.h"
#include "../objects/abstractsimulationobjectmodel.h"
#include "../objects/simulationobjectoptionswidget.h"
#include "../objects/simulationobjectlistwidget.h"

#include <kddockwidgets-qt6/kddockwidgets/DockWidget.h>
#include <kddockwidgets-qt6/kddockwidgets/core/DockWidget.h>

#include <QInputDialog>

ViewManager::ViewManager(MainWindow *parent)
    : QObject{parent}
{
    ModeManager *modeMgr = mainWin()->modeMgr();

    connect(modeMgr, &ModeManager::modeChanged,
            this, &ViewManager::onFileModeChanged);

    CircuitListModel *circuitList = modeMgr->circuitList();
    connect(circuitList, &CircuitListModel::nodeEditRequested,
            this, &ViewManager::nodeEditRequested);
    connect(circuitList, &CircuitListModel::cableEditRequested,
            this, &ViewManager::cableEditRequested);
}

ViewManager::~ViewManager()
{
    closeAll();
}

void ViewManager::setActiveCircuit(CircuitWidget *w)
{
    if(w && w == mActiveCircuitView)
        return;

    mActiveCircuitView = w;
    if(!mActiveCircuitView && !mCircuitViews.isEmpty())
        mActiveCircuitView = mCircuitViews.begin().key();
}

static std::pair<QString, QString> getCircuitUniqueName(CircuitWidget *w)
{
    QString name = QLatin1String("null");
    QString title = ViewManager::tr("Empty");

    auto scene = w->scene();
    if(scene)
        title = scene->circuitSheetName();

    name = QLatin1String("%1_(%2)").arg(title).arg(w->uniqueNum());
    title = QLatin1String("%1 (%2)").arg(title).arg(w->uniqueNum());

    return {name, title};
}

void ViewManager::updateDockName(CircuitWidget *w)
{
    auto dock = mCircuitViews.value(w, nullptr);
    if(!dock)
        return;

    auto namePair = getCircuitUniqueName(w);
    dock->dockWidget()->setUniqueName(namePair.first);
    dock->setTitle(namePair.second);
}

MainWindow *ViewManager::mainWin()
{
    return static_cast<MainWindow *>(parent());
}

int ViewManager::getUniqueNum(CircuitScene *scene, CircuitWidget *self) const
{
    int result = 1;

    while(true)
    {
        bool duplicate = false;

        for(auto it : mCircuitViews.asKeyValueRange())
        {
            CircuitWidget *w = it.first;
            if(w == self)
                continue;

            if(w->scene() != scene)
                continue;

            if(w->uniqueNum() == result)
            {
                duplicate = true;
                break;
            }
        }

        if(!duplicate)
            return result;

        result++; // Go to next num
    }

    return 0;
}

CircuitWidget *ViewManager::addCircuitView(CircuitScene *scene, bool forceNew)
{
    if(!forceNew)
    {
        for(auto it : mCircuitViews.asKeyValueRange())
        {
            CircuitWidget *w = it.first;
            if(w->scene() == scene)
            {
                w->raise();
                return w;
            }
        }
    }

    // Create new view
    CircuitWidget *w = new CircuitWidget(this);
    w->setScene(scene, false);

    auto namePair = getCircuitUniqueName(w);

    DockWidget *dock = new DockWidget(namePair.first,
                                      KDDockWidgets::DockWidgetOption_DeleteOnClose);
    dock->setWidget(w);
    dock->setTitle(namePair.second);

    mainWin()->addDockWidget(dock, KDDockWidgets::Location_OnRight);

    connect(w, &CircuitWidget::destroyed,
            this, &ViewManager::onCircuitViewDestroyed);

    mCircuitViews.insert(w, dock);

    if(!mActiveCircuitView)
        mActiveCircuitView = w;

    return w;
}

void ViewManager::showCircuitSceneEdit(CircuitScene *scene)
{
    // Raise existing edit window if present
    for(auto it : mCircuitEdits.asKeyValueRange())
    {
        if(it.first == scene)
        {
            it.second->raise();
            return;
        }
    }

    // Create new edit window
    CircuitSceneOptionsWidget *w = new CircuitSceneOptionsWidget(scene);
    DockWidget *dock = new DockWidget(scene->circuitSheetName(),
                                      KDDockWidgets::DockWidgetOption_DeleteOnClose);
    dock->setWidget(w);

    auto updateDock = [dock, scene]()
    {
        QString name = tr("Edit Circuit %1").arg(scene->circuitSheetName());
        dock->setTitle(name);
        dock->dockWidget()->setUniqueName(name);
    };

    connect(scene, &CircuitScene::nameChanged,
            dock, updateDock);
    connect(scene, &CircuitScene::destroyed,
            dock, &QWidget::close);
    connect(dock, &QObject::destroyed,
            this, [this, scene]()
    {
        mCircuitEdits.remove(scene);
    });

    updateDock();

    // By default open as floating window
    dock->open();

    mCircuitEdits.insert(scene, dock);
}

void ViewManager::showObjectEdit(AbstractSimulationObject *item)
{
    // Raise existing edit window if present
    for(auto it : mObjectEdits.asKeyValueRange())
    {
        if(it.first == item)
        {
            it.second->raise();
            return;
        }
    }

    SimulationObjectFactory *factory = mainWin()->modeMgr()->objectFactory();

    // Create new edit window
    SimulationObjectOptionsWidget *w = factory->createEditWidget(nullptr, item);

    DockWidget *dock = new DockWidget(item->name(),
                                      KDDockWidgets::DockWidgetOption_DeleteOnClose);
    dock->setWidget(w);

    auto updateDock = [dock, item, w, factory]()
    {
        QString name = tr("Edit %1 %2")
                .arg(factory->prettyName(item->getType()),
                     item->name());
        dock->setTitle(name);
        dock->dockWidget()->setUniqueName(w->uniqueId());
    };

    connect(w, &SimulationObjectOptionsWidget::uniqueIdChanged,
            dock, updateDock);
    connect(item, &AbstractSimulationObject::destroyed,
            dock, &QWidget::close);
    connect(dock, &QObject::destroyed,
            this, [this, item]()
    {
        mObjectEdits.remove(item);
    });

    // Set initial title
    updateDock();

    mObjectEdits.insert(item, dock);

    // By default open as floating window
    dock->open();
}

void ViewManager::closeAllEditDocks()
{
    auto circuitEditsCopy = mCircuitEdits;
    qDeleteAll(circuitEditsCopy);
    mCircuitEdits.clear();

    auto objectEditsCopy = mObjectEdits;
    qDeleteAll(objectEditsCopy);
    mObjectEdits.clear();
}

void ViewManager::closeAllFileSpecificDocks()
{
    closeAllEditDocks();

    auto circuitViewsCopy = mCircuitViews;
    qDeleteAll(circuitViewsCopy);
    mCircuitViews.clear();
}

void ViewManager::closeAll()
{
    closeAllFileSpecificDocks();

    delete mCircuitListViewDock;

    qDeleteAll(mObjectListDocks);
    mObjectListDocks.clear();
}

void ViewManager::showCircuitListView()
{
    if(mCircuitListViewDock)
    {
        mCircuitListViewDock->raise();
        mCircuitListViewDock->activateWindow();
        return;
    }

    CircuitListModel *circuitList = mainWin()->modeMgr()->circuitList();
    CircuitListWidget *circuitListView = new CircuitListWidget(this, circuitList);

    mCircuitListViewDock = new DockWidget(QLatin1String("circuit_list"),
                                          KDDockWidgets::DockWidgetOption_DeleteOnClose);
    mCircuitListViewDock->setWidget(circuitListView);
    mCircuitListViewDock->setTitle(tr("Circuit Sheets"));

    mainWin()->addDockWidget(mCircuitListViewDock, KDDockWidgets::Location_OnLeft);
}

void ViewManager::showObjectListView(const QString &objType)
{
    // Raise existing edit window if present
    DockWidget *dock = mObjectListDocks.value(objType, nullptr);
    if(dock)
    {
        dock->raise();
        dock->activateWindow();
        return;
    }

    AbstractSimulationObjectModel *model = mainWin()->modeMgr()->modelForType(objType);
    if(!model)
        return;

    // Create new list widget
    SimulationObjectListWidget *w =
            new SimulationObjectListWidget(this, model);

    dock = new DockWidget(QLatin1String("list_%1").arg(model->getObjectType()),
                          KDDockWidgets::DockWidgetOption_DeleteOnClose);
    dock->setWidget(w);

    SimulationObjectFactory *factory = mainWin()->modeMgr()->objectFactory();
    dock->setTitle(tr("%1 List").arg(factory->prettyName(objType)));

    connect(dock, &QObject::destroyed,
            this, [this, objType]()
    {
        mObjectListDocks.remove(objType);
    });

    mObjectListDocks.insert(objType, dock);

    // By default add to Main Window
    mainWin()->addDockWidget(dock,
                             KDDockWidgets::Location_OnLeft);
}

void ViewManager::startEditNEwCableOnActiveView()
{
    if(!mActiveCircuitView)
        return;

    if(mainWin()->modeMgr()->mode() != FileMode::Editing)
        return;

    mActiveCircuitView->scene()->startEditNewCable();
}

void ViewManager::addNodeToActiveView(const QString &nodeType)
{
    if(!mActiveCircuitView)
        return;

    if(mainWin()->modeMgr()->mode() != FileMode::Editing)
        return;

    const auto editFactory = mainWin()->modeMgr()->circuitFactory();
    mActiveCircuitView->addNodeToCenter(editFactory, nodeType);
}

void ViewManager::onCircuitViewDestroyed(QObject *obj)
{
    CircuitWidget *w = static_cast<CircuitWidget *>(obj);

    // Widget is destroyed by dock being closed
    // So dock deletes itself already
    mCircuitViews.remove(w);

    // Unset from active if it was us
    // Do it AFTER removing from mCircuitViews
    // Because setActiveCircuit() will try to use first
    // available view as replacment
    if(w == mActiveCircuitView)
        setActiveCircuit(nullptr);
}

void ViewManager::nodeEditRequested(AbstractNodeGraphItem *item)
{
    Q_ASSERT_X(mActiveCircuitView,
               "nodeEditRequested", "no active view");

    DockWidget *dock = mCircuitViews.value(mActiveCircuitView);

    // Allow delete or custom node options
    auto editFactory = mainWin()->modeMgr()->circuitFactory();
    editFactory->editItem(dock, item);
}

void ViewManager::cableEditRequested(CableGraphItem *item)
{
    Q_ASSERT_X(mActiveCircuitView,
               "nodeEditRequested", "no active view");

    // Allow delete or modify path
    auto editFactory = mainWin()->modeMgr()->circuitFactory();
    editFactory->editCable(mActiveCircuitView, item);
}

void ViewManager::onFileModeChanged(FileMode mode, FileMode oldMode)
{
    Q_UNUSED(mode);
    if(oldMode == FileMode::Editing)
        closeAllEditDocks();
}

CircuitWidget *ViewManager::activeCircuitView() const
{
    return mActiveCircuitView;
}
