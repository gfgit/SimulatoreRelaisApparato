/**
 * src/network/view/remotesessionlistwidget.cpp
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

#include "remotesessionlistwidget.h"

#include "../../views/viewmanager.h"
#include "../../views/modemanager.h"

#include "../remotemanager.h"
#include "../remotesessionsmodel.h"

#include <QTableView>
#include <QPushButton>

#include <QInputDialog>
#include <QMessageBox>

#include <QBoxLayout>

RemoteSessionListWidget::RemoteSessionListWidget(ViewManager *viewMgr, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(viewMgr)
{
    RemoteManager *remoteMgr = mViewMgr->modeMgr()->getRemoteManager();
    mModel = remoteMgr->remoteSessionsModel();

    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add"));
    addBut->setToolTip(tr("Add Remote Session"));
    butLay->addWidget(addBut);

    remBut = new QPushButton(tr("Remove"));
    remBut->setToolTip(tr("Remove Remote Session"));
    butLay->addWidget(remBut);

    mView = new QTableView;
    mView->setModel(mModel);
    lay->addWidget(mView);
    mView->resizeColumnsToContents();

    connect(addBut, &QPushButton::clicked,
            this, &RemoteSessionListWidget::addRemoteSession);
    connect(remBut, &QPushButton::clicked,
            this, &RemoteSessionListWidget::removeRemoteSession);

    connect(mViewMgr->modeMgr(), &ModeManager::modeChanged,
            this, &RemoteSessionListWidget::onFileModeChanged);

    onFileModeChanged(mViewMgr->modeMgr()->mode());
}

void RemoteSessionListWidget::resizeColumns()
{
    mView->resizeColumnsToContents();
}

void RemoteSessionListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void RemoteSessionListWidget::addRemoteSession()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    RemoteManager *remoteMgr = mViewMgr->modeMgr()->getRemoteManager();
    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(this,
                                     tr("New Remote Session"),
                                     first ?
                                         tr("Name:") :
                                         tr("<b>%1</b> already added.<br>"
                                            "Set another name:").arg(name),
                                     QLineEdit::Normal,
                                     name).trimmed();
        if(name.isEmpty())
            return;

        if(!remoteMgr->getRemoteSession(name))
            break;

        first = false;
    }

    remoteMgr->addRemoteSession(name);
}

void RemoteSessionListWidget::removeRemoteSession()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    RemoteSession *remoteSession = mModel->getRemoteSessionAt(idx.row());
    if(!remoteSession)
        return;

    QString name = mModel->data(idx.siblingAtColumn(0),
                                Qt::DisplayRole).toString();

    int ret = QMessageBox::question(this,
                                    tr("Delete Remote Session?"),
                                    tr("Are you sure to delete <b>%1</b>?<br>"
                                       "This will reset all remote objects set to this session.").arg(name));
    if(ret == QMessageBox::Yes)
    {
        RemoteManager *remoteMgr = mViewMgr->modeMgr()->getRemoteManager();
        remoteMgr->removeRemoteSession(remoteSession);
    }
}
