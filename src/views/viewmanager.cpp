#include "viewmanager.h"

#include "circuitwidget.h"
#include "circuitlistmodel.h"
#include "circuitlistwidget.h"
#include "../graph/circuitscene.h"

#include "../mainwindow.h"
#include "modemanager.h"
#include "../nodes/edit/nodeeditfactory.h"

#include <kddockwidgets-qt6/kddockwidgets/DockWidget.h>
#include <kddockwidgets-qt6/kddockwidgets/core/DockWidget.h>

#include <QInputDialog>

#include <QDebug>

ViewManager::ViewManager(MainWindow *parent)
    : QObject{parent}
{
    CircuitListModel *circuitList = mainWin()->modeMgr()->circuitList();
    connect(circuitList, &CircuitListModel::nodeEditRequested,
            this, &ViewManager::nodeEditRequested);
    connect(circuitList, &CircuitListModel::cableEditRequested,
            this, &ViewManager::cableEditRequested);
}

void ViewManager::setActiveCircuit(CircuitWidget *w)
{
    if(w && w == mActiveCircuitView)
        return;

    if(w)
        qDebug() << "Set ACTIVE:" << mCircuitViews.value(w)->dockWidget()->uniqueName();
    else
        qDebug() << "Set ACTIVE: NULL";

    mActiveCircuitView = w;
    if(!mActiveCircuitView && !mCircuitViews.isEmpty())
        mActiveCircuitView = mCircuitViews.begin().key();
}

static QString getCircuitUniqueName(CircuitWidget *w)
{
    QString name = QLatin1String("null");

    auto scene = w->scene();
    if(scene)
        name = scene->circuitSheetName();

    return QLatin1String("%1_(%2)").arg(name).arg(w->uniqueNum());
}

void ViewManager::updateDockName(CircuitWidget *w)
{
    auto dock = mCircuitViews.value(w, nullptr);
    if(!dock)
        return;

    dock->dockWidget()->setUniqueName(getCircuitUniqueName(w));
}

MainWindow *ViewManager::mainWin()
{
    return static_cast<MainWindow *>(parent());
}

int ViewManager::getUniqueNum(CircuitScene *scene) const
{
    int result = 1;

    while(true)
    {
        bool duplicate = false;

        for(auto it : mCircuitViews.asKeyValueRange())
        {
            CircuitWidget *w = it.first;
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

    DockWidget *dock = new DockWidget(getCircuitUniqueName(w));
    dock->setWidget(w);
    dock->setAttribute(Qt::WA_DeleteOnClose);
    mainWin()->addDockWidget(dock, KDDockWidgets::Location_OnRight);

    connect(w, &CircuitWidget::destroyed,
            this, &ViewManager::onCircuitViewDestroyed);

    mCircuitViews.insert(w, dock);

    if(!mActiveCircuitView)
        mActiveCircuitView = w;

    return w;
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

    mCircuitListViewDock = new DockWidget(QLatin1String("circuit_list"));
    mCircuitListViewDock->setWidget(circuitListView);
    mCircuitListViewDock->setAttribute(Qt::WA_DeleteOnClose);

    mainWin()->addDockWidget(mCircuitListViewDock, KDDockWidgets::Location_OnLeft);
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
    if(w == mActiveCircuitView)
        setActiveCircuit(nullptr);

    // Widget is destroyed by dock being closed
    // So dock deletes itself already
    mCircuitViews.remove(w);
}

void ViewManager::nodeEditRequested(AbstractNodeGraphItem *item)
{
    Q_ASSERT(mActiveCircuitView);

    DockWidget *dock = mCircuitViews.value(mActiveCircuitView);
    QString name = dock->dockWidget()->uniqueName();
    qDebug() << "NODE EDIT: view" << name;

    // Allow delete or custom node options
    auto editFactory = mainWin()->modeMgr()->circuitFactory();
    editFactory->editItem(dock, item);
}

void ViewManager::cableEditRequested(CableGraphItem *item)
{
    Q_ASSERT(mActiveCircuitView);

    // Allow delete or modify path
    auto editFactory = mainWin()->modeMgr()->circuitFactory();
    editFactory->editCable(mActiveCircuitView, item);
}

CircuitWidget *ViewManager::activeCircuitView() const
{
    return mActiveCircuitView;
}
