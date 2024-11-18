/**
 * src/objects/simulationobjectlineedit.cpp
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

#include "simulationobjectlineedit.h"

#include "abstractsimulationobject.h"
#include "abstractsimulationobjectmodel.h"

#include "../views/modemanager.h"

#include <QCompleter>
#include <QAbstractProxyModel>
#include <QStringListModel>

#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>

SimulationObjectLineEdit::SimulationObjectLineEdit(ModeManager *mgr, const QStringList &types, QWidget *parent)
    : QWidget(parent)
    , modeMgr(mgr)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(QMargins());

    typesModel = new QStringListModel(types, this);

    mTypesCombo = new QComboBox;
    mTypesCombo->setModel(typesModel);
    lay->addWidget(mTypesCombo);

    mCompleter = new QCompleter;
    mCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mCompleter->setFilterMode(Qt::MatchContains);
    mCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    mLineEdit = new QLineEdit;
    mLineEdit->setCompleter(mCompleter);
    lay->addWidget(mLineEdit);

    // Default to first type
    setType(0);

    connect(mTypesCombo, &QComboBox::activated,
            this, &SimulationObjectLineEdit::setType);

    connect(mCompleter, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        setObject(mModel->objectAt(idx.row()));
    });

    connect(mCompleter, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        QAbstractProxyModel *m = static_cast<QAbstractProxyModel *>(mCompleter->completionModel());
        const QModelIndex sourceIdx = m->mapToSource(idx);

        setObject(mModel->objectAt(sourceIdx.row()));
    });
}

void SimulationObjectLineEdit::setObject(AbstractSimulationObject *newObject)
{
    if(mObject == newObject)
        return;

    mObject = newObject;

    if(mObject)
    {
        setType(typesModel->stringList().indexOf(mObject->getType()));

        if(mLineEdit->text() != mObject->name())
            mLineEdit->setText(mObject->name());
    }
    else
    {
        setType(0);

        if(!mLineEdit->text().isEmpty())
            mLineEdit->setText(QString());
    }

    emit objectChanged(mObject);
}

void SimulationObjectLineEdit::setType(int idx)
{
    if(idx < 0)
        idx = 0; // Default to first

    const QString type = typesModel->stringList().at(idx);

    if(mObject && mObject->getType() != type)
    {
        // Reset old object
        setObject(nullptr); // Recursion
    }

    mTypesCombo->setCurrentIndex(idx);

    mModel = modeMgr->modelForType(type);
    mCompleter->setModel(mModel);
}
