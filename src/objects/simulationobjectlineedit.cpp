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

#include "simulationobjectmultitypemodel.h"

#include "simulationobjectfactory.h"

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
    , mTypes(types)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(QMargins());

    SimulationObjectFactory *factory = modeMgr->objectFactory();

    // Add "Auto" for when there are more possibilities
    const bool addAutoMode = mTypes.size() > 1;
    if(addAutoMode)
        mTypes.prepend(QString()); // Auto

    // Fill combo with pretty names
    QStringList prettyTypes;
    prettyTypes.reserve(mTypes.size());
    for(const QString& type : std::as_const(mTypes))
    {
        prettyTypes.append(factory->prettyName(type));
    }

    if(addAutoMode)
        prettyTypes[0] = tr("Auto");

    mTypesModel = new QStringListModel(prettyTypes, this);

    mTypesCombo = new QComboBox;
    mTypesCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mTypesCombo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    mTypesCombo->setModel(mTypesModel);
    lay->addWidget(mTypesCombo);

    mCompleter = new QCompleter;
    mCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mCompleter->setFilterMode(Qt::MatchContains);
    mCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    mLineEdit = new QLineEdit;
    mLineEdit->setCompleter(mCompleter);
    lay->addWidget(mLineEdit);

    // Default to Auto type
    setType(0);

    connect(mTypesCombo, &QComboBox::activated,
            this, &SimulationObjectLineEdit::setType);

    connect(mCompleter, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        if(mMultiModel)
            setObject(mMultiModel->objectAt(idx.row()));
        else
            setObject(mModel->objectAt(idx.row()));
    });

    connect(mCompleter, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        QAbstractProxyModel *m = static_cast<QAbstractProxyModel *>(mCompleter->completionModel());
        const QModelIndex sourceIdx = m->mapToSource(idx);

        if(mMultiModel)
            setObject(mMultiModel->objectAt(sourceIdx.row()));
        else
            setObject(mModel->objectAt(sourceIdx.row()));
    });

    if(mTypes.size() == 1)
        mTypesCombo->hide(); // No need to show if type cannot be changed

    connect(mLineEdit, &QLineEdit::returnPressed,
            [this]()
    {
        // Allow un-set current object
        if(mLineEdit->text().isEmpty())
            setObject(nullptr);
    });
}

void SimulationObjectLineEdit::setObject(AbstractSimulationObject *newObject)
{
    if(mObject == newObject)
        return;

    mObject = newObject;

    if(mObject)
    {
        if(!mMultiModel)
            setType(mTypes.indexOf(mObject->getType()));

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
        idx = 0; // Default to Auto

    const QString type = mTypes.at(idx);

    if(type.isEmpty() && !mMultiModel)
    {
        // Auto mode
        QStringList types = mTypes;
        types.removeFirst();
        mMultiModel = new SimulationObjectMultiTypeModel(modeMgr,
                                                         types, this);
    }
    else if(!type.isEmpty() && mMultiModel)
    {
        delete mMultiModel;
        mMultiModel = nullptr;
    }

    mTypesCombo->setCurrentIndex(idx);

    if(mMultiModel)
    {
        mCompleter->setModel(mMultiModel);
    }
    else
    {
        mModel = modeMgr->modelForType(type);
        mCompleter->setModel(mModel);
    }
}
