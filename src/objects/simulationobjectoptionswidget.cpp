/**
 * src/objects/simulationobjectoptionswidget.cpp
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

#include "simulationobjectoptionswidget.h"

#include "abstractsimulationobjectmodel.h"
#include "abstractsimulationobject.h"

#include <QFormLayout>
#include <QLineEdit>

SimulationObjectOptionsWidget::SimulationObjectOptionsWidget(AbstractSimulationObject *object, QWidget *parent)
    : QWidget{parent}
    , mObject(object)
{
    QFormLayout *lay = new QFormLayout(this);

    // Name
    mNameEdit = new QLineEdit;
    mNameEdit->setPlaceholderText(tr("Unique name"));
    mNameEdit->setText(mObject->name());
    lay->addRow(tr("Name:"), mNameEdit);

    normalEditPalette = mNameEdit->palette();

    // Description
    mDescriptionEdit = new QLineEdit;
    mDescriptionEdit->setPlaceholderText(tr("Description"));
    mDescriptionEdit->setText(mObject->description());
    lay->addRow(tr("Description:"), mDescriptionEdit);

    connect(mNameEdit, &QLineEdit::editingFinished,
            this, &SimulationObjectOptionsWidget::setName);
    connect(mNameEdit, &QLineEdit::returnPressed,
            this, &SimulationObjectOptionsWidget::setName);
    connect(mNameEdit, &QLineEdit::textEdited,
            this, &SimulationObjectOptionsWidget::onNameTextEdited);

    connect(mObject, &AbstractSimulationObject::nameChanged,
            this, &SimulationObjectOptionsWidget::onNameChanged);

    connect(mDescriptionEdit, &QLineEdit::textEdited,
            this, [this]()
    {
        mObject->setDescription(mDescriptionEdit->text());
    });

    connect(mObject, &AbstractSimulationObject::descriptionChanged,
            this, [this]()
    {
        if(mObject->description() == mDescriptionEdit->text())
            return;
        mDescriptionEdit->setText(mObject->description());
    });

    // Update name
    onNameChanged();
}

void SimulationObjectOptionsWidget::addCustomWidget(QWidget *w)
{
    static_cast<QFormLayout *>(layout())->addRow(w);
}

QString SimulationObjectOptionsWidget::uniqueId() const
{
    return QLatin1String("edit_%1").arg(mObject->uniqueId());
}

void SimulationObjectOptionsWidget::setName()
{
    QString newName = mNameEdit->text().trimmed();
    if(!newName.isEmpty() && mObject->setName(newName))
    {
        // Name is valid
        return;
    }

    // Name is not valid, go back to old name
    mNameEdit->setText(mObject->name());
    setNameValid(true);
}

void SimulationObjectOptionsWidget::onNameTextEdited()
{
    QString newName = mNameEdit->text().trimmed();

    bool valid = true;
    if(newName != mObject->name())
        valid = mObject->model()->isNameAvailable(newName);

    setNameValid(valid);
}

void SimulationObjectOptionsWidget::onNameChanged()
{
    if(mObject->name() != mNameEdit->text())
        mNameEdit->setText(mObject->name());

    setWindowTitle(tr("Edit %1 (%2)").arg(mObject->name(), mObject->getType()));

    emit uniqueIdChanged(uniqueId());
}

void SimulationObjectOptionsWidget::setNameValid(bool valid)
{
    if(valid)
    {
        mNameEdit->setPalette(normalEditPalette);
        mNameEdit->setToolTip(QString());
    }
    else
    {
        // Red text
        QPalette p = mNameEdit->palette();
        p.setColor(QPalette::Text, Qt::red);
        mNameEdit->setPalette(p);

        mNameEdit->setToolTip(tr("Name already exists"));
    }
}
