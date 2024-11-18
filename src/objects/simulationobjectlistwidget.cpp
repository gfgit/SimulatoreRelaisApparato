/**
 * src/objects/simulationobjectlistwidget.cpp
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

#include "simulationobjectlistwidget.h"

#include "abstractsimulationobjectmodel.h"
#include "abstractsimulationobject.h"

#include "simulationobjectfactory.h"

#include "../views/viewmanager.h"
#include "../views/modemanager.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QPushButton>

#include <QInputDialog>
#include <QMessageBox>

#include <QSortFilterProxyModel>

#include <QMenu>
#include <QAction>

SimulationObjectListWidget::SimulationObjectListWidget(ViewManager *mgr, AbstractSimulationObjectModel *model, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(mgr)
    , mModel(model)
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    const QString prettyName = mModel->getObjectPrettyName();

    addBut = new QPushButton(tr("Add %1").arg(prettyName));
    remBut = new QPushButton(tr("Remove %1").arg(prettyName));

    butLay->addWidget(addBut);
    butLay->addWidget(remBut);

    mView = new QTableView;
    lay->addWidget(mView);

    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSourceModel(mModel);
    mProxyModel->setSortRole(Qt::DisplayRole);
    mProxyModel->sort(0);

    mView->setModel(mProxyModel);
    mView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mModel->modeMgr(), &ModeManager::modeChanged,
            this, &SimulationObjectListWidget::onFileModeChanged);

    connect(addBut, &QPushButton::clicked,
            this, &SimulationObjectListWidget::addObject);
    connect(remBut, &QPushButton::clicked,
            this, &SimulationObjectListWidget::removeCurrentObject);

    connect(mView, &QTableView::customContextMenuRequested,
            this, &SimulationObjectListWidget::showViewContextMenu);

    onFileModeChanged(mModel->modeMgr()->mode());
}

AbstractSimulationObjectModel *SimulationObjectListWidget::model() const
{
    return mModel;
}

void SimulationObjectListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void SimulationObjectListWidget::addObject()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    const QString prettyName = mModel->getObjectPrettyName();

    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(this,
                                     tr("New %1").arg(prettyName),
                                     first ?
                                         tr("Choose name:") :
                                         tr("Name is not available.\n"
                                            "Choose another name:"),
                                     QLineEdit::Normal,
                                     name);
        if(name.isEmpty())
            return;

        if(mModel->isNameAvailable(name))
            break;

        first = false;
    }

    SimulationObjectFactory *factory = mModel->modeMgr()->objectFactory();
    AbstractSimulationObject *item = factory->createItem(mModel);

    item->setName(name);
    mModel->addObject(item);
}

void SimulationObjectListWidget::removeCurrentObject()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    idx = mProxyModel->mapToSource(idx);
    if(!idx.isValid())
        return;

    AbstractSimulationObject *item = mModel->objectAt(idx.row());
    if(!item)
        return;

    const QString prettyName = mModel->getObjectPrettyName();

    int ret = QMessageBox::question(this,
                                    tr("Delete %1?").arg(prettyName),
                                    tr("Are you sure to delete <b>%1</b>?")
                                    .arg(item->name()));
    if(ret == QMessageBox::Yes)
    {
        mModel->removeObject(item);
    }
}

void SimulationObjectListWidget::showViewContextMenu(const QPoint &pos)
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QPointer<QMenu> menu = new QMenu(this);

    QModelIndex idx = mView->indexAt(pos);
    idx = mProxyModel->mapToSource(idx);

    AbstractSimulationObject *item = mModel->objectAt(idx.row());
    if(!item)
        return;

    QAction *actionEdit = menu->addAction(tr("Edit"));
    QAction *ret = menu->exec(mView->viewport()->mapToGlobal(pos));
    if(ret == actionEdit)
        mViewMgr->showObjectEdit(item);
}