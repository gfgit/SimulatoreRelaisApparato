/**
 * src/objects/interfaces/mechanical/view/mechanicalconditionsview.cpp
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

#include "mechanicalconditionsview.h"
#include "mechanicalconditionsitemdelegate.h"

#include "../model/mechanicalconditionsmodel.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QPushButton>

MechanicalConditionsView::MechanicalConditionsView(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add Condition"));
    remBut = new QPushButton(tr("Remove Condition"));

    butLay->addWidget(addBut);
    butLay->addWidget(remBut);

    mView = new QTableView;
    lay->addWidget(mView);

    mView->setItemDelegate(new MechanicalConditionsItemDelegate(this));

    connect(addBut, &QPushButton::clicked,
            this, &MechanicalConditionsView::addCondition);
    connect(remBut, &QPushButton::clicked,
            this, &MechanicalConditionsView::removeCurrentCondition);
}

MechanicalConditionsModel *MechanicalConditionsView::model() const
{
    return mModel;
}

void MechanicalConditionsView::setModel(MechanicalConditionsModel *newModel)
{
    if(mModel == newModel)
        return;

    mModel = newModel;
    mView->setModel(mModel);
}

void MechanicalConditionsView::addCondition()
{
    if(!mModel)
        return;

    QModelIndex idx = mView->currentIndex();

    int row = 0;
    if(idx.isValid())
        row = idx.row() + 1;

    // Add after current index
    mModel->instertConditionAt(row);
}

void MechanicalConditionsView::removeCurrentCondition()
{
    if(!mModel)
        return;

    QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    mModel->removeConditionAt(idx.row());
}
