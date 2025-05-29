/**
 * src/network/view/replicaslistwidget.cpp
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

#include "replicaslistwidget.h"

#include "../../views/viewmanager.h"
#include "../../views/modemanager.h"

#include "../remotemanager.h"
#include "../remotesessionsmodel.h"
#include "../replicaobjectmanager.h"

#include "../replicasmodel.h"
#include "replicasdelegate.h"

#include "../../objects/simulationobjectlineedit.h"
#include "../../objects/simulationobjectfactory.h"

#include <QTableView>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>

#include <QPointer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <QBoxLayout>
#include <QFormLayout>

ReplicasListWidget::ReplicasListWidget(ViewManager *viewMgr, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(viewMgr)
{
    RemoteManager *remoteMgr = mViewMgr->modeMgr()->getRemoteManager();
    connect(remoteMgr, &RemoteManager::remoteSessionRemoved,
            this, &ReplicasListWidget::onRemoteSessionRemoved);

    ReplicaObjectManager *replicaMgr = remoteMgr->replicaMgr();
    mModel = replicaMgr->replicasModel();

    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add"));
    addBut->setToolTip(tr("Add Replica"));
    butLay->addWidget(addBut);

    remBut = new QPushButton(tr("Remove"));
    remBut->setToolTip(tr("Remove Replica"));
    butLay->addWidget(remBut);

    mView = new QTableView;
    mView->setModel(mModel);
    mView->setItemDelegate(new ReplicasDelegate(replicaMgr->remoteMgr()->remoteSessionsModel(), this));
    lay->addWidget(mView);

    connect(addBut, &QPushButton::clicked,
            this, &ReplicasListWidget::addReplica);
    connect(remBut, &QPushButton::clicked,
            this, &ReplicasListWidget::removeReplica);

    connect(mViewMgr->modeMgr(), &ModeManager::modeChanged,
            this, &ReplicasListWidget::onFileModeChanged);

    onFileModeChanged(mViewMgr->modeMgr()->mode());
}

void ReplicasListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void ReplicasListWidget::addReplica()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    ReplicaObjectManager *replicaMgr = mViewMgr->modeMgr()->getRemoteManager()->replicaMgr();
    AbstractSimulationObject *replicaObj = nullptr;

    const auto replicaTypes = mViewMgr->modeMgr()->objectFactory()->replicaTypes();
    if(replicaTypes.isEmpty())
        return;

    QPointer<QDialog> dlg = new QDialog(this);
    QFormLayout *lay = new QFormLayout(dlg);

    SimulationObjectLineEdit *objEdit = new SimulationObjectLineEdit(mViewMgr,
                                                                     replicaTypes);
    objEdit->setDefaultType(mLastObjType);
    lay->addRow(tr("Replica:"), objEdit);

    QCheckBox *sessionCheck = new QCheckBox(tr("Use Remote Session"));
    sessionCheck->setChecked(mUseRemoteSession);

    RemoteSessionsModel *sessionsModel = mViewMgr->modeMgr()->getRemoteManager()->remoteSessionsModel();
    QComboBox *sessionCombo = new QComboBox;
    sessionCombo->setModel(sessionsModel);
    lay->addRow(sessionCheck, sessionCombo);

    if(mLastSession)
        sessionCombo->setCurrentIndex(sessionsModel->rowForRemoteSession(mLastSession));

    auto updSession = [sessionCombo, sessionCheck]()
    {
        sessionCombo->setEnabled(sessionCheck->isChecked());
    };

    connect(sessionCheck, &QCheckBox::toggled,
            sessionCombo, updSession);
    updSession();

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lay->addRow(box);

    connect(box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    while(dlg)
    {
        if(dlg->exec() != QDialog::Accepted)
            break;

        replicaObj = objEdit->getObject();
        if(!replicaObj)
            continue;

        if(!replicaMgr->addReplicaObject(replicaObj))
        {
            objEdit->setObject(nullptr);
            continue;
        }

        RemoteSession *newSession = sessionsModel->getRemoteSessionAt(sessionCombo->currentIndex());
        if(sessionCheck->isChecked())
        {
            if(!replicaMgr->setReplicaObjectSession(replicaObj, newSession, QString()))
            {
                QMessageBox::warning(this, tr("Remote Session"),
                                     tr("Could not set selected remote session."));
            }
        }

        // Success
        mLastObjType = objEdit->getDefaultType();
        mUseRemoteSession = sessionCheck->isChecked();
        mLastSession = newSession;

        break;
    }

    delete dlg;
}

void ReplicasListWidget::removeReplica()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    mModel->removeAt(idx.row());
}

void ReplicasListWidget::onRemoteSessionRemoved(RemoteSession *remoteSession)
{
    if(mLastSession == remoteSession)
        mLastSession = nullptr;
}
