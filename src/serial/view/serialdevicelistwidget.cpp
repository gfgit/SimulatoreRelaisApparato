/**
 * src/serial/view/serialdevicelistwidget.cpp
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

#include "serialdevicelistwidget.h"

#include "../../views/viewmanager.h"
#include "../../views/modemanager.h"

#include "../serialmanager.h"
#include "../serialdevicesmodel.h"

#include <QTableView>
#include <QPushButton>

#include <QInputDialog>
#include <QMessageBox>

#include <QBoxLayout>

SerialDeviceListWidget::SerialDeviceListWidget(ViewManager *viewMgr, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(viewMgr)
{
    SerialManager *serialMgr = mViewMgr->modeMgr()->getSerialManager();
    mModel = serialMgr->devicesModel();

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
            this, &SerialDeviceListWidget::addSerialDevice);
    connect(remBut, &QPushButton::clicked,
            this, &SerialDeviceListWidget::removeSerialDevice);

    connect(mViewMgr->modeMgr(), &ModeManager::modeChanged,
            this, &SerialDeviceListWidget::onFileModeChanged);

    onFileModeChanged(mViewMgr->modeMgr()->mode());
}

void SerialDeviceListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void SerialDeviceListWidget::addSerialDevice()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    SerialManager *serialMgr = mViewMgr->modeMgr()->getSerialManager();
    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(this,
                                     tr("New Serial Device"),
                                     first ?
                                         tr("Name:") :
                                         tr("<b>%1</b> already added.<br>"
                                            "Set another name:").arg(name),
                                     QLineEdit::Normal,
                                     name).trimmed();
        if(name.isEmpty())
            return;

        if(!serialMgr->getDevice(name))
            break;

        first = false;
    }

    serialMgr->addDevice(name);
}

void SerialDeviceListWidget::removeSerialDevice()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    SerialDevice *SerialDevice = mModel->getSerialDeviceAt(idx.row());
    if(!SerialDevice)
        return;

    QString name = mModel->data(idx.siblingAtColumn(0),
                                Qt::DisplayRole).toString();

    int ret = QMessageBox::question(this,
                                    tr("Delete Serial Device?"),
                                    tr("Are you sure to delete <b>%1</b>?<br>"
                                       "This will reset all remote objects set to this device.").arg(name));
    if(ret == QMessageBox::Yes)
    {
        SerialManager *serialMgr = mViewMgr->modeMgr()->getSerialManager();
        serialMgr->removeDevice(SerialDevice);
    }
}
