/**
 * src/views/layoutloader.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#include "layoutloader.h"

#include "viewmanager.h"
#include "modemanager.h"

#include "../objects/abstractsimulationobjectmodel.h"

#include "../circuits/view/circuitlistmodel.h"
#include "../circuits/view/circuitwidget.h"
#include "../circuits/view/circuitsview.h"

#include "../panels/view/panellistmodel.h"
#include "../panels/view/panelwidget.h"
#include "../panels/view/panelview.h"

#include <kddockwidgets/Config.h>
#include <kddockwidgets/DockWidget.h>
#include <kddockwidgets/LayoutSaver.h>

#include <QJsonObject>

static constexpr QLatin1String IDKey("id");
static constexpr QLatin1String ZoomKey("zoom");
static constexpr QLatin1String ScrollXKey("scroll_x");
static constexpr QLatin1String ScrollYKey("scroll_y");
static constexpr QLatin1String StatusBarKey("status_bar");

void LayoutLoader::loadLayout(const QByteArray &data)
{
    // Close all before loading new layout
    ViewManager::self()->closeAll();

    setDeleteOnClose(false);

    // NOTE: we skip restoring MainWindow geometry because:
    // 1 - it's glitchy. Window must then be manually un-maximized/re-maximized
    // 2 - User could have changed screen setup since last layout save
    KDDockWidgets::LayoutSaver sa(KDDockWidgets::RestoreOption_RelativeToMainWindow);
    sa.restoreLayout(data);

    setDeleteOnClose(true);
}

QByteArray LayoutLoader::saveLayout()
{
    KDDockWidgets::LayoutSaver sa;
    return sa.serializeLayout();
}

QJsonObject LayoutLoader::saveLayoutConfig()
{
    QJsonObject layout;

    // Custom zoom config for circuits/panels views
    ViewManager *viewMgr = ViewManager::self();

    // Circuits
    for(auto it = viewMgr->mCircuitViews.cbegin(), e = viewMgr->mCircuitViews.cend();
        it != e; it++)
    {
        CircuitWidget *w = it.key();

        QJsonObject obj;
        obj[IDKey] = it.value()->uniqueName();
        obj[ZoomKey] = w->circuitsView()->zoomFactor();

        const QPoint scrollPos = w->circuitsView()->getScrollPosition();
        obj[ScrollXKey] = scrollPos.x();
        obj[ScrollYKey] = scrollPos.y();

        obj[StatusBarKey] = w->isStatusBarVisible();

        layout[it.value()->uniqueName()] = obj;
    }

    // Panels
    for(auto it = viewMgr->mPanelViews.cbegin(), e = viewMgr->mPanelViews.cend();
        it != e; it++)
    {
        PanelWidget *w = it.key();

        QJsonObject obj;
        obj[IDKey] = it.value()->uniqueName();
        obj[ZoomKey] = w->panelView()->zoomFactor();

        const QPoint scrollPos = w->panelView()->getScrollPosition();
        obj[ScrollXKey] = scrollPos.x();
        obj[ScrollYKey] = scrollPos.y();

        obj[StatusBarKey] = w->isStatusBarVisible();

        layout[it.value()->uniqueName()] = obj;
    }

    return layout;
}

void LayoutLoader::loadLayoutConfig(const QJsonObject &layout)
{
    // Custom zoom config for circuits/panels views
    ViewManager *viewMgr = ViewManager::self();

    // Circuits
    for(auto it = viewMgr->mCircuitViews.cbegin(), e = viewMgr->mCircuitViews.cend();
        it != e; it++)
    {
        CircuitWidget *w = it.key();

        const QJsonValue v = layout.value(it.value()->uniqueName());
        if(!v.isObject())
            continue;

        const QJsonObject obj = v.toObject();

        w->circuitsView()->setZoom(obj.value(ZoomKey).toDouble(1.0));
        w->setStatusBarVisible(obj.value(StatusBarKey).toBool(false));

        QPoint scrollPos;
        scrollPos.setX(obj.value(ScrollXKey).toInt());
        scrollPos.setY(obj.value(ScrollYKey).toInt());

        w->circuitsView()->setScrollPosition(scrollPos);
    }

    // Panels
    for(auto it = viewMgr->mPanelViews.cbegin(), e = viewMgr->mPanelViews.cend();
        it != e; it++)
    {
        PanelWidget *w = it.key();

        const QJsonValue v = layout.value(it.value()->uniqueName());
        if(!v.isObject())
            continue;

        const QJsonObject obj = v.toObject();

        w->panelView()->setZoom(obj.value(ZoomKey).toDouble(1.0));
        w->setStatusBarVisible(obj.value(StatusBarKey).toBool(false));

        QPoint scrollPos;
        scrollPos.setX(obj.value(ScrollXKey).toInt());
        scrollPos.setY(obj.value(ScrollYKey).toInt());

        w->panelView()->setScrollPosition(scrollPos);
    }
}

inline void setFlag(ViewManager::DockWidget *dock, bool value)
{
    if(!dock)
        return;

    auto opt = dock->options();
    opt.setFlag(KDDockWidgets::DockWidgetOption_DeleteOnClose, value);
    dock->setOptions(opt);

    if(value && !dock->isOpen())
        dock->deleteLater();
}

KDDockWidgets::QtWidgets::DockWidget *LayoutLoader::createDockWidget(const QString &name)
{
    ViewManager *viewMgr = ViewManager::self();

    // Circuits
    if(name == QLatin1String("circuit_list"))
    {
        viewMgr->showCircuitListView();
        setFlag(viewMgr->mCircuitListViewDock.data(), false);
        return viewMgr->mCircuitListViewDock.data();
    }

    if(name.startsWith(QLatin1String("prop_circuit_")))
    {
        QString sceneName = name.mid(13);
        if(sceneName.isEmpty())
            return nullptr;

        CircuitScene *scene = viewMgr->modeMgr()->circuitList()->sceneByName(sceneName);
        if(!scene)
            return nullptr;

        viewMgr->showCircuitSceneProperties(scene);
        setFlag(viewMgr->mCircuitEdits.value(scene), false);
        return viewMgr->mCircuitEdits.value(scene);
    }

    if(name.startsWith(QLatin1String("circuit_")))
    {
        if(name.endsWith("_null") || !name.endsWith(')'))
            return nullptr;

        QStringView str(name);
        const int lastParentesis = str.lastIndexOf('(');
        if(lastParentesis < 0)
            return nullptr;

        QStringView numView = str.mid(lastParentesis + 1,
                                      str.size() - 1 - lastParentesis - 1);
        bool ok = false;
        const int uniqueNum = numView.toInt(&ok);
        if(!ok || numView.isEmpty())
            return nullptr;

        QStringView sceneName = str.mid(8, lastParentesis - 8 - 1);
        if(sceneName.isEmpty())
            return nullptr;

        CircuitScene *scene = viewMgr->modeMgr()->circuitList()->sceneByName(sceneName.toString());
        if(!scene)
            return nullptr;

        CircuitWidget *w = viewMgr->addCircuitView(scene, true);
        w->setUniqueNum(uniqueNum);
        viewMgr->updateDockName(w);

        setFlag(viewMgr->mCircuitViews.value(w), false);
        return viewMgr->mCircuitViews.value(w);
    }

    // Panels
    if(name == QLatin1String("panel_list"))
    {
        viewMgr->showPanelListView();
        setFlag(viewMgr->mPanelListViewDock.data(), false);
        return viewMgr->mPanelListViewDock.data();
    }

    if(name.startsWith(QLatin1String("prop_panel_")))
    {
        QString sceneName = name.mid(11);
        if(sceneName.isEmpty())
            return nullptr;

        PanelScene *scene = viewMgr->modeMgr()->panelList()->sceneByName(sceneName);
        if(!scene)
            return nullptr;

        viewMgr->showPanelSceneProperties(scene);
        setFlag(viewMgr->mPanelEdits.value(scene), false);
        return viewMgr->mPanelEdits.value(scene);
    }

    if(name.startsWith(QLatin1String("panel_")))
    {
        if(name.endsWith("_null") || !name.endsWith(')'))
            return nullptr;

        QStringView str(name);
        const int lastParentesis = str.lastIndexOf('(');
        if(lastParentesis < 0)
            return nullptr;

        QStringView numView = str.mid(lastParentesis + 1,
                                      str.size() - 1 - lastParentesis - 1);
        bool ok = false;
        const int uniqueNum = numView.toInt(&ok);
        if(!ok || numView.isEmpty())
            return nullptr;

        QStringView sceneName = str.mid(6, lastParentesis - 6 - 1);
        if(sceneName.isEmpty())
            return nullptr;

        PanelScene *scene = viewMgr->modeMgr()->panelList()->sceneByName(sceneName.toString());
        if(!scene)
            return nullptr;

        PanelWidget *w = viewMgr->addPanelView(scene, true);
        w->setUniqueNum(uniqueNum);
        viewMgr->updateDockName(w);

        setFlag(viewMgr->mPanelViews.value(w), false);
        return viewMgr->mPanelViews.value(w);
    }

    // Objects
    if(name.startsWith(QLatin1String("list_")))
    {
        QString objType = name.mid(5);

        if(objType.isEmpty())
            return nullptr;

        viewMgr->showObjectListView(objType);

        setFlag(viewMgr->mObjectListDocks.value(objType), false);
        return viewMgr->mObjectListDocks.value(objType);
    }

    if(name.startsWith(QLatin1String("edit_")))
    {
        QStringView str(name);
        QStringView objectId = str.mid(5);
        int dotIdx = objectId.indexOf('.');
        if(dotIdx < 0)
            return nullptr;

        QStringView objType = objectId.first(dotIdx);
        QStringView objName = objectId.mid(dotIdx + 1);

        if(objType.isEmpty() || objName.isEmpty())
            return nullptr;

        auto model = viewMgr->modeMgr()->modelForType(objType.toString());
        if(!model)
            return nullptr;

        auto obj = model->getObjectByName(objName.toString());
        if(!obj)
            return nullptr;

        viewMgr->showObjectProperties(obj);

        setFlag(viewMgr->mObjectEdits.value(obj), false);
        return viewMgr->mObjectEdits.value(obj);
    }

    return nullptr;
}

KDDockWidgets::Core::DockWidget *LayoutLoader::dockWidgetFactoryFunc(const QString &name)
{
    auto *dock = createDockWidget(name);

    if(dock)
    {
        setFlag(dock, false);
        return dock->dockWidget();
    }

    return nullptr;
}

void LayoutLoader::setDeleteOnClose(bool value)
{
    ViewManager *viewMgr = ViewManager::self();

    // Circuits
    setFlag(viewMgr->mCircuitListViewDock, value);

    for(ViewManager::DockWidget *dock : viewMgr->mCircuitViews)
    {
        setFlag(dock, value);
    }

    for(ViewManager::DockWidget *dock : viewMgr->mCircuitEdits)
    {
        setFlag(dock, value);
    }

    // Panels
    setFlag(viewMgr->mPanelListViewDock, value);

    for(ViewManager::DockWidget *dock : viewMgr->mPanelViews)
    {
        setFlag(dock, value);
    }

    for(ViewManager::DockWidget *dock : viewMgr->mPanelEdits)
    {
        setFlag(dock, value);
    }

    // Objects
    for(ViewManager::DockWidget *dock : viewMgr->mObjectListDocks)
    {
        setFlag(dock, value);
    }

    for(ViewManager::DockWidget *dock : viewMgr->mObjectEdits)
    {
        setFlag(dock, value);
    }
}

void LayoutLoader::registerLoader()
{
    KDDockWidgets::Config::self().setDockWidgetFactoryFunc(&LayoutLoader::dockWidgetFactoryFunc);
}
