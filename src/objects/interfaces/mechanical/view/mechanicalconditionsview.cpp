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
#include <QTreeView>
#include <QPushButton>

MechanicalConditionsView::MechanicalConditionsView(ModeManager *mgr, QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add Condition"));
    remBut = new QPushButton(tr("Remove Condition"));

    addOrBut = new QPushButton(tr("Add OR"));
    addAndBut = new QPushButton(tr("Add AND"));

    butLay->addWidget(addBut);
    butLay->addWidget(remBut);
    butLay->addWidget(addOrBut);
    butLay->addWidget(addAndBut);

    mView = new QTreeView;
    lay->addWidget(mView);

    mView->setItemDelegate(new MechanicalConditionsItemDelegate(mgr, this));

    connect(addBut, &QPushButton::clicked,
            this, &MechanicalConditionsView::addCondition);
    connect(addOrBut, &QPushButton::clicked,
            this, &MechanicalConditionsView::addCondition);
    connect(addAndBut, &QPushButton::clicked,
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
    mView->expandAll();
}

void MechanicalConditionsView::expandAll()
{
    if(!mModel)
        return;
    mView->expandAll();
}

void MechanicalConditionsView::addCondition()
{
    if(!mModel)
        return;

    MechanicalConditionsModel::Item::Type type =
            MechanicalConditionsModel::Item::Type::ExactPos;

    if(sender() == addBut)
    {
        type = MechanicalConditionsModel::Item::Type::ExactPos;
    }
    else if(sender() == addOrBut)
    {
        type = MechanicalConditionsModel::Item::Type::Or;
    }
    else if(sender() == addAndBut)
    {
        type = MechanicalConditionsModel::Item::Type::And;
    }
    else
    {
        return;
    }

    QModelIndex idx = mView->currentIndex();
    idx = mModel->instertConditionAt(idx, type);
    mView->expand(idx);
}

void MechanicalConditionsView::removeCurrentCondition()
{
    if(!mModel)
        return;

    const QModelIndex idx = mView->currentIndex();
    mModel->removeConditionAt(idx);
}
