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

#include <QCompleter>
#include <QAbstractProxyModel>

SimulationObjectLineEdit::SimulationObjectLineEdit(AbstractSimulationObjectModel *m, QWidget *parent)
    : QLineEdit(parent)
    , mModel(m)
{
    QCompleter *c = new QCompleter;
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setFilterMode(Qt::MatchContains);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->setModel(mModel);
    setCompleter(c);

    connect(c, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        setObject(mModel->objectAt(idx.row()));
    });

    connect(c, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this, c](const QModelIndex& idx)
    {
        QAbstractProxyModel *m = static_cast<QAbstractProxyModel *>(c->completionModel());
        const QModelIndex sourceIdx = m->mapToSource(idx);

        setObject(mModel->objectAt(sourceIdx.row()));
    });
}

void SimulationObjectLineEdit::setObject(AbstractSimulationObject *newObject)
{
    if(mObject == newObject)
        return;
    mObject = newObject;

    if(text() != mObject->name())
        setText(mObject->name());

    emit objectChanged(mObject);
}
