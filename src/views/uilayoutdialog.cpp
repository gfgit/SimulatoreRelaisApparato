/**
 * src/views/uilayoutdialog.cpp
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

#include "uilayoutdialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QTableView>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "viewmanager.h"
#include "modemanager.h"
#include "uilayoutsmodel.h"

#include <QInputDialog>
#include <QMessageBox>

UILayoutDialog::UILayoutDialog(ViewManager *viewMgr, QWidget *parent)
    : QDialog{parent}
    , mViewMgr(viewMgr)
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    mView = new QTableView;
    mView->setModel(mViewMgr->getLayoutsModel());
    mView->setSelectionMode(QTableView::SingleSelection);
    mView->setSelectionBehavior(QTableView::SelectRows);
    mView->setEditTriggers(QTableView::NoEditTriggers);
    lay->addWidget(mView);

    QVBoxLayout *butLay = new QVBoxLayout;
    lay->addLayout(butLay);

    mAddBut = new QPushButton(tr("Add Layout"));
    mAddBut->setToolTip(tr("To use a layout double click on its row."));

    mRemBut = new QPushButton(tr("Remove Layout"));

    mEditCurrentBut = new QPushButton(tr("Edit"));
    mEditCurrentBut->setToolTip(tr("Change selected layout name"));

    mStoreCurrentBut = new QPushButton(tr("Store Layout"));
    mStoreCurrentBut->setToolTip(tr("Store current layout with selected layout name.\n"
                                    "It will overwrite previous saved layout of same name."));

    mSetLoadAtStart = new QPushButton(tr("Load on start"));

    mLastLayoutAtStartCB = new QCheckBox(tr("Load last layout at start"));

    butLay->addWidget(mAddBut);
    butLay->addWidget(mRemBut);
    butLay->addWidget(mEditCurrentBut);
    butLay->addWidget(mStoreCurrentBut);
    butLay->addWidget(mSetLoadAtStart);
    butLay->addWidget(mLastLayoutAtStartCB);


    mRemBut->setEnabled(false);
    mEditCurrentBut->setEnabled(false);
    mStoreCurrentBut->setEnabled(false);
    mSetLoadAtStart->setEnabled(false);

    connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]()
    {
        const bool hasSel = mView->selectionModel()->hasSelection();
        mRemBut->setEnabled(hasSel);
        mEditCurrentBut->setEnabled(hasSel);
        mStoreCurrentBut->setEnabled(hasSel);
        mSetLoadAtStart->setEnabled(hasSel);
    });

    connect(mAddBut, &QPushButton::clicked,
            this, &UILayoutDialog::onAdd);
    connect(mRemBut, &QPushButton::clicked,
            this, &UILayoutDialog::onRemove);
    connect(mStoreCurrentBut, &QPushButton::clicked,
            this, &UILayoutDialog::onStoreCurrent);
    connect(mEditCurrentBut, &QPushButton::clicked,
            this, &UILayoutDialog::onEditName);
    connect(mSetLoadAtStart, &QPushButton::clicked,
            this, &UILayoutDialog::onSetLoadAtStart);

    connect(mLastLayoutAtStartCB, &QCheckBox::toggled,
            this, &UILayoutDialog::onLastLayoutCBChanged);

    connect(mView, &QTableView::activated,
            this, &UILayoutDialog::onActivated);

    connect(mViewMgr->getLayoutsModel(), &UILayoutsModel::layoutToLoadAtStartChanged,
            this, &UILayoutDialog::onStartLayoutChanged);
}

void UILayoutDialog::onAdd()
{
    if(mViewMgr->modeMgr()->filePath().isEmpty())
    {
        QMessageBox::warning(this,
                             tr("Save first!"),
                             tr("This is a new file.\n"
                                "To save layouts, first save the file."));
        return;
    }

    auto model = mViewMgr->getLayoutsModel();
    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(this,
                                     tr("New Layout"),
                                     first ?
                                         tr("Choose name:") :
                                         tr("Name is not available.\n"
                                            "Choose another name:"),
                                     QLineEdit::Normal,
                                     name);
        if(name.isEmpty())
            return;

        if(!model->layoutExists(name))
            break;

        first = false;
    }

    model->addLayout(name);
    model->saveLayout(name);
}

void UILayoutDialog::onRemove()
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    auto model = mViewMgr->getLayoutsModel();
    const QString layoutName = model->layoutNameAt(idx.row());
    int ret = QMessageBox::question(this,
                                    tr("Remove Layout?"),
                                    tr("Remote <b>%1</b> layout?").arg(layoutName));
    if(ret != QMessageBox::Yes)
        return;

    model->removeLayoutAt(idx.row());
}

void UILayoutDialog::onStoreCurrent()
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    auto model = mViewMgr->getLayoutsModel();
    const QString layoutName = model->layoutNameAt(idx.row());
    model->saveLayout(layoutName);
}

void UILayoutDialog::onEditName()
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    auto model = mViewMgr->getLayoutsModel();
    const QString layoutName = model->layoutNameAt(idx.row());

    QString newName = layoutName;

    bool first = true;
    while(true)
    {
        newName = QInputDialog::getText(this,
                                     tr("Edit Layout"),
                                     first ?
                                         tr("Choose name:") :
                                         tr("Name is not available.\n"
                                            "Choose another name:"),
                                     QLineEdit::Normal,
                                     newName);
        if(newName.isEmpty() || newName == layoutName)
            return;

        if(!model->layoutExists(newName))
            break;

        first = false;
    }

    model->renameLayout(layoutName, newName);
}

void UILayoutDialog::onSetLoadAtStart()
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    auto model = mViewMgr->getLayoutsModel();
    const QString layoutName = model->layoutNameAt(idx.row());
    model->setLayoutToLoadAtStart(layoutName);
}

void UILayoutDialog::onActivated(const QModelIndex &idx)
{
    if(!idx.isValid())
        return;

    auto model = mViewMgr->getLayoutsModel();
    const QString layoutName = model->layoutNameAt(idx.row());
    model->loadLayout(layoutName);

    // Close dialog
    accept();
}

void UILayoutDialog::onLastLayoutCBChanged()
{
    auto model = mViewMgr->getLayoutsModel();
    const bool loadLast = mLastLayoutAtStartCB->isChecked();

    QString layoutAtStart;
    if(!loadLast)
    {
        layoutAtStart = model->layoutNameAt(0);
    }

    mLastLayoutAtStartCB->setChecked(layoutAtStart.isEmpty());
    model->setLayoutToLoadAtStart(layoutAtStart);
}

void UILayoutDialog::onStartLayoutChanged()
{
    auto model = mViewMgr->getLayoutsModel();
    QString layoutAtStart = model->layoutToLoadAtStart();
    mLastLayoutAtStartCB->setChecked(layoutAtStart.isEmpty());
}
