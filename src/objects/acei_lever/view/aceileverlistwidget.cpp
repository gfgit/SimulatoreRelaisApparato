/**
 * src/objects/acei_lever/view/aceileverlistwidget.cpp
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

#include "aceileverlistwidget.h"

#include "../model/aceilevermodel.h"
#include "../model/aceileverobject.h"

#include "../../../views/viewmanager.h"
#include "../../../views/modemanager.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QPushButton>

#include <QInputDialog>
#include <QMessageBox>

#include <QSortFilterProxyModel>

ACEILeverListWidget::ACEILeverListWidget(ViewManager *mgr, ACEILeverModel *model, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(mgr)
    , mModel(model)
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add Lever"));
    remBut = new QPushButton(tr("Remove Lever"));

    butLay->addWidget(addBut);
    butLay->addWidget(remBut);

    mView = new QTableView;
    lay->addWidget(mView);

    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSourceModel(mModel);
    mProxyModel->setSortRole(Qt::DisplayRole);
    mProxyModel->sort(0);

    mView->setModel(mProxyModel);

    connect(mModel->modeMgr(), &ModeManager::modeChanged,
            this, &ACEILeverListWidget::onFileModeChanged);

    connect(addBut, &QPushButton::clicked,
            this, &ACEILeverListWidget::addLever);
    connect(remBut, &QPushButton::clicked,
            this, &ACEILeverListWidget::removeCurrentLever);

    onFileModeChanged(mModel->modeMgr()->mode());
}

ACEILeverModel *ACEILeverListWidget::model() const
{
    return mModel;
}

void ACEILeverListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void ACEILeverListWidget::addLever()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(this,
                                     tr("New ACEI Lever"),
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

    ACEILeverObject *r = new ACEILeverObject(mModel);
    r->setName(name);
    mModel->addLever(r);
}

void ACEILeverListWidget::removeCurrentLever()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    idx = mProxyModel->mapToSource(idx);
    if(!idx.isValid())
        return;

    ACEILeverObject *r = mModel->leverAt(idx.row());
    if(!r)
        return;

    int ret = QMessageBox::question(this,
                                    tr("Delete Lever?"),
                                    tr("Are you sure to delete <b>%1</b>?")
                                    .arg(r->name()));
    if(ret == QMessageBox::Yes)
    {
        mModel->removeLever(r);
    }
}
