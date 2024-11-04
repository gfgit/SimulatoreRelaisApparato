#include "viewmanager.h"

#include "../mainwindow.h"
#include "modemanager.h"
#include "../nodes/edit/nodeeditfactory.h"

#include "circuitwidget.h"
#include "circuitlistmodel.h"
#include "../graph/circuitscene.h"

#include "circuitlistwidget.h"

#include "relaislistwidget.h"

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
        QString name = tr("Edit %1").arg(scene->circuitSheetName());
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

void ViewManager::closeAllEditDocks()
{
    auto circuitEditsCopy = mCircuitEdits;
    qDeleteAll(circuitEditsCopy);
    mCircuitEdits.clear();
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
    delete mRelaisListViewDock;
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

    mainWin()->addDockWidget(mCircuitListViewDock, KDDockWidgets::Location_OnLeft);
}

void ViewManager::showRelayListView()
{
    if(mRelaisListViewDock)
    {
        mRelaisListViewDock->raise();
        mRelaisListViewDock->activateWindow();
        return;
    }

    RelaisModel *relaisList = mainWin()->modeMgr()->relaisModel();
    RelaisListWidget *relaisListView = new RelaisListWidget(this, relaisList);

    mRelaisListViewDock = new DockWidget(QLatin1String("relais_list"),
                                          KDDockWidgets::DockWidgetOption_DeleteOnClose);
    mRelaisListViewDock->setWidget(relaisListView);

    mainWin()->addDockWidget(mRelaisListViewDock, KDDockWidgets::Location_OnLeft);
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
