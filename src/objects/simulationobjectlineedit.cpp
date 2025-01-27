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
#include "../views/viewmanager.h"

#include <QCompleter>
#include <QAbstractProxyModel>
#include <QStringListModel>

#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

SimulationObjectLineEdit::SimulationObjectLineEdit(ViewManager *viewMgr,
                                                   const QStringList &types,
                                                   QWidget *parent)
    : QWidget(parent)
    , mViewMgr(viewMgr)
    , mModeMgr(mViewMgr->modeMgr())
    , mTypes(types)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(QMargins());

    SimulationObjectFactory *factory = mModeMgr->objectFactory();

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

    mEditObjectBut = new QPushButton(tr("..."));
    mEditObjectBut->setEnabled(false);
    mEditObjectBut->setToolTip(tr("Show Object Properties"));
    lay->addWidget(mEditObjectBut);

    mNewObjectBut = new QPushButton(tr("New"));
    mNewObjectBut->setToolTip(tr("Create new Object"));
    lay->addWidget(mNewObjectBut);

    // Default to Auto type
    setType(0);

    connect(mTypesCombo, &QComboBox::activated,
            this, &SimulationObjectLineEdit::setType);

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

    connect(mLineEdit, &QLineEdit::textEdited,
            [this]()
    {
        mObjectIsDirty = true;
    });

    connect(mLineEdit, &QLineEdit::returnPressed,
            [this]()
    {
        // Allow un-set current object
        if(mLineEdit->text().isEmpty())
        {
            setObject(nullptr);
        }
        else if(mObjectIsDirty)
        {
            // Text was edited without chosing an item from completer popup
            // Try to find it by name
            if(mMultiModel)
                setObject(mMultiModel->getObjectByName(mLineEdit->text()));
            else
                setObject(mModel->getObjectByName(mLineEdit->text()));
        }

        updateObjectName();
    });

    connect(mEditObjectBut, &QPushButton::clicked,
            this, &SimulationObjectLineEdit::editCurrentObject);

    connect(mNewObjectBut, &QPushButton::clicked,
            this, &SimulationObjectLineEdit::onNewObject);
}

void SimulationObjectLineEdit::setObjectEditAllowed(bool allow)
{
    if(mObjectEditAllowed == allow)
        return;

    mObjectEditAllowed = allow;

    mEditObjectBut->setVisible(mObjectEditAllowed);
    mNewObjectBut->setVisible(mObjectEditAllowed && !mObject);
}

void SimulationObjectLineEdit::setObject(AbstractSimulationObject *newObject)
{
    mObjectIsDirty = false;

    if(mObject == newObject)
        return;

    if(mObject)
    {
        disconnect(mObject, &AbstractSimulationObject::nameChanged,
                   this, &SimulationObjectLineEdit::updateObjectName);
    }

    mObject = newObject;

    if(mObject)
    {
        if(!mMultiModel)
            setType(mTypes.indexOf(mObject->getType()));

        connect(mObject, &AbstractSimulationObject::nameChanged,
                this, &SimulationObjectLineEdit::updateObjectName);
    }
    else
    {
        setType(0);
    }

    updateObjectName();

    mEditObjectBut->setEnabled(mObject != nullptr);
    mNewObjectBut->setVisible(mObjectEditAllowed && !mObject);

    emit objectChanged(mObject);
}

void SimulationObjectLineEdit::editCurrentObject()
{
    if(!mObject)
        return;

    mViewMgr->showObjectEdit(mObject);

    emit editRequested();
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
        mMultiModel = new SimulationObjectMultiTypeModel(mModeMgr,
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
        mModel = mModeMgr->modelForType(type);
        mCompleter->setModel(mModel);
    }
}

void SimulationObjectLineEdit::updateObjectName()
{
    if(mObject)
    {
        if(mLineEdit->text() != mObject->name())
            mLineEdit->setText(mObject->name());
    }
    else
    {
        if(!mLineEdit->text().isEmpty())
            mLineEdit->setText(QString());
    }
}

void SimulationObjectLineEdit::onNewObject()
{
    QString type = mTypes.at(mTypesCombo->currentIndex());

    if(type.isEmpty() && mTypes.size() > 2)
    {
        // We are in "Auto" mode and there are more types
        // So we cannot know which type of object user wants to create
        // Popup type combo to let user explicitly choose type.
        mTypesCombo->showPopup();
        return;
    }

    if(type.isEmpty())
        type = mTypes.value(1, QString()); // Default to first type

    if(type.isEmpty())
        return;

    AbstractSimulationObject *obj = mViewMgr->createNewObjectDlg(type, this);
    if(!obj)
        return;

    setObject(obj);
}
