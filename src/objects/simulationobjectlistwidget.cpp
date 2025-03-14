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
#include "simulationobjectoptionswidget.h"

#include "simulationobjectcopyhelper.h"

#include "../views/viewmanager.h"
#include "../views/modemanager.h"

#include "../utils/jsondiff.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QPushButton>

#include <QInputDialog>
#include <QMessageBox>

#include <QSortFilterProxyModel>
#include <QItemSelectionModel>

#include <QMenu>
#include <QAction>

#include <QJsonObject>

#include <QDialog>
#include <QDialogButtonBox>
#include <QPointer>

#include <QKeyEvent>

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
    batchEditBut = new QPushButton(tr("Batch Edit"));

    butLay->addWidget(addBut);
    butLay->addWidget(remBut);
    butLay->addWidget(batchEditBut);
    batchEditBut->setVisible(false);
    batchEditBut->setToolTip(tr("Edit multiple objects togheter.\n"
                                "Edit one object, changed settings will be applied to"
                                "all selected objects."));

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
    connect(batchEditBut, &QPushButton::clicked,
            this, &SimulationObjectListWidget::onBatchEdit);

    connect(mView, &QTableView::customContextMenuRequested,
            this, &SimulationObjectListWidget::showViewContextMenu);

    connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &SimulationObjectListWidget::onSelectionChanged);

    onFileModeChanged(mModel->modeMgr()->mode());
}

AbstractSimulationObjectModel *SimulationObjectListWidget::model() const
{
    return mModel;
}

AbstractSimulationObject *SimulationObjectListWidget::addObjectHelper(AbstractSimulationObjectModel *model, QWidget *parent)
{
    if(model->modeMgr()->mode() != FileMode::Editing)
        return nullptr;

    const QString prettyName = model->getObjectPrettyName();

    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(parent,
                                     tr("New %1").arg(prettyName),
                                     first ?
                                         tr("New %1\n"
                                            "Choose name:").arg(prettyName) :
                                         tr("New %1\n"
                                            "Name is not available.\n"
                                            "Choose another name:").arg(prettyName),
                                     QLineEdit::Normal,
                                     name);
        if(name.isEmpty())
            return nullptr;

        if(model->isNameAvailable(name))
            break;

        first = false;
    }

    SimulationObjectFactory *factory = model->modeMgr()->objectFactory();
    AbstractSimulationObject *item = factory->createItem(model);

    item->setName(name);
    model->addObject(item);

    return item;
}

void SimulationObjectListWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->matches(QKeySequence::Copy))
    {
        copySelectedObjectToClipboard();
        ev->accept();
        return;
    }
    else if(ev->matches(QKeySequence::Paste))
    {
        pasteFromClipboard();
        ev->accept();
        return;
    }

    QWidget::keyPressEvent(ev);
}

bool hasMultipleRows(const QItemSelection& sel)
{
    int selectedRowsCount = 0;
    for(const QItemSelectionRange& r : sel)
    {
        if(r.isEmpty() || !r.isValid())
            continue;

        selectedRowsCount += r.height();
        if(selectedRowsCount > 1)
            return true;
    }

    return false;
}

void SimulationObjectListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
    batchEditBut->setEnabled(canEdit);
    batchEditBut->setVisible(canEdit &&
                             hasMultipleRows(mView->selectionModel()->selection()));
}

void SimulationObjectListWidget::addObject()
{
    AbstractSimulationObject *item = addObjectHelper(mModel, this);
    if(!item)
        return;

    mView->clearSelection();

    const int row = mModel->rowForObject(item);
    if(row < 0)
        return;

    const QModelIndex source = mModel->index(row, 0);
    const QModelIndex dest = mProxyModel->mapFromSource(source);

    mView->scrollTo(dest);
    mView->selectionModel()->select(dest, QItemSelectionModel::Select);
    mView->setCurrentIndex(dest);
}

void SimulationObjectListWidget::removeCurrentObject()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    const QVector<AbstractSimulationObject *> selectedObjs = getSelectedObjects();
    if(selectedObjs.isEmpty())
        return;

    int ret = QMessageBox::question(this,
                                    tr("Delete?"),
                                    tr("Are you sure to delete selected object?"));
    if(ret == QMessageBox::Yes)
    {
        for(AbstractSimulationObject *item : selectedObjs)
            mModel->removeObject(item);
    }
}

void SimulationObjectListWidget::showViewContextMenu(const QPoint &pos)
{
    QPointer<QMenu> menu = new QMenu(this);

    QModelIndex idx = mView->indexAt(pos);
    idx = mProxyModel->mapToSource(idx);

    AbstractSimulationObject *item = mModel->objectAt(idx.row());
    if(!item)
        return;

    QAction *actionBatchEdit = menu->addAction(tr("Batch Edit"));
    actionBatchEdit->setVisible(batchEditBut->isEnabled() && hasMultipleRows(mView->selectionModel()->selection()));

    QAction *actionProperties = menu->addAction(tr("Properties"));
    QAction *ret = menu->exec(mView->viewport()->mapToGlobal(pos));

    if(ret == actionProperties)
        mViewMgr->showObjectProperties(item);
    else if(ret == actionBatchEdit)
        onBatchEdit();
}

void SimulationObjectListWidget::onSelectionChanged()
{
    // Visible if can edit and multiple rows are selected
    batchEditBut->setVisible(batchEditBut->isEnabled() &&
                             hasMultipleRows(mView->selectionModel()->selection()));
}

void SimulationObjectListWidget::onBatchEdit()
{
    const QVector<AbstractSimulationObject *> selectedObjs = getSelectedObjects();
    if(selectedObjs.isEmpty())
        return;

    const QModelIndex curIdx = mProxyModel->mapToSource(mView->selectionModel()->currentIndex());
    AbstractSimulationObject *curObj = mModel->objectAt(curIdx.row());

    if(!curObj || !selectedObjs.contains(curObj))
        curObj = selectedObjs.first();

    QJsonObject origSettings;
    curObj->saveToJSON(origSettings);

    SimulationObjectFactory *factory = mModel->modeMgr()->objectFactory();

    QPointer<QDialog> dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Batch Object Edit"));

    QVBoxLayout *lay = new QVBoxLayout(dlg);

    // Create edit widget
    SimulationObjectOptionsWidget *w = factory->createEditWidget(nullptr, curObj, mViewMgr);
    lay->addWidget(w);

    QDialogButtonBox *dlgBut = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                    Qt::Horizontal);
    lay->addWidget(dlgBut);

    connect(dlgBut, &QDialogButtonBox::accepted,
            dlg, &QDialog::accept);
    connect(dlgBut, &QDialogButtonBox::rejected,
            dlg, &QDialog::reject);

    const int ret = dlg->exec();
    if(dlg)
        delete dlg;

    if(ret != QDialog::Accepted)
    {
        // Restore original settings
        curObj->loadFromJSON(origSettings, LoadPhase::Creation);
        curObj->loadFromJSON(origSettings, LoadPhase::AllCreated);
        return;
    }

    QJsonObject newSettings;
    curObj->saveToJSON(newSettings);

    QString namePrefix;
    QString nameSuffix;

    QStringList modifiedKeys = JSONDiff::checkDifferencesTopLevel(origSettings,
                                                                  newSettings,
                                                                  namePrefix,
                                                                  nameSuffix);

    for(AbstractSimulationObject *obj : std::as_const(selectedObjs))
    {
        if(obj == curObj)
            continue;

        QJsonObject settings;
        obj->saveToJSON(settings);

        JSONDiff::applyDiff(settings, newSettings, modifiedKeys,
                            namePrefix, nameSuffix);

        obj->loadFromJSON(settings, LoadPhase::Creation);
        obj->loadFromJSON(settings, LoadPhase::AllCreated);
    }
}

bool SimulationObjectListWidget::copySelectedObjectToClipboard()
{
    QVector<AbstractSimulationObject *> selectedObjs = getSelectedObjects();
    if(selectedObjs.isEmpty())
        return false;

    QJsonObject rootObj;
    rootObj["objects"] = SimulationObjectCopyHelper::copyObjects(mModel->modeMgr(),
                                                                 selectedObjs);

    SimulationObjectCopyHelper::copyToClipboard(rootObj);
    return true;
}

bool SimulationObjectListWidget::pasteFromClipboard()
{
    QJsonObject rootObj;
    if(!SimulationObjectCopyHelper::getPasteDataFromClipboard(rootObj))
        return false;

    const QJsonObject objPool = rootObj.value("objects").toObject();
    SimulationObjectCopyHelper::pasteObjects(mModel->modeMgr(), objPool);
    return true;
}

QVector<AbstractSimulationObject *> SimulationObjectListWidget::getSelectedObjects()
{
    QVector<AbstractSimulationObject *> selectedObjs;

    const QItemSelection sel = mView->selectionModel()->selection();

    for(const QItemSelectionRange& r : sel)
    {
        if(r.isEmpty() || !r.isValid())
            continue;

        for(int i = r.top(); i <= r.bottom(); i++)
        {
            const QModelIndex dest = mProxyModel->index(i, 0);
            const QModelIndex source = mProxyModel->mapToSource(dest);
            if(!source.isValid())
                continue;

            AbstractSimulationObject *obj = mModel->objectAt(source.row());
            selectedObjs.append(obj);
        }
    }

    return selectedObjs;
}
