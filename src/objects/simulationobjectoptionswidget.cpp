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

#include "simulationobjectnodesmodel.h"

#include "../views/viewmanager.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QTabWidget>

#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QTableView>

#include <QGuiApplication>
#include <QEvent>

SimulationObjectOptionsWidget::SimulationObjectOptionsWidget(AbstractSimulationObject *object, ViewManager *viewMgr, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(viewMgr)
    , mObject(object)
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    tabWidget = new QTabWidget;
    mainLay->addWidget(tabWidget);

    // Properties Tab
    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, tr("Properties"));

    QWidget *viewport = new QWidget;
    scrollArea->setWidget(viewport);

    QFormLayout *lay = new QFormLayout(viewport);

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

    // Nodes Tab

    mNodesModel = new SimulationObjectNodesModel(mViewMgr, this);
    mNodesModel->setObject(mObject);

    mNodesView = new QTableView;
    mNodesView->setModel(mNodesModel);
    connect(mNodesView, &QTableView::clicked,
            this, &SimulationObjectOptionsWidget::onNodeClicked);

    tabWidget->addTab(mNodesView, tr("Nodes"));

    // Update name
    onNameChanged();
}

void SimulationObjectOptionsWidget::addCustomWidget(QWidget *w)
{
    static_cast<QFormLayout *>(scrollArea->widget()->layout())->addRow(w);
}

QString SimulationObjectOptionsWidget::uniqueId() const
{
    return QLatin1String("edit_%1").arg(mObject->uniqueId());
}

void SimulationObjectOptionsWidget::fixScrollingChildrenInScrollArea()
{
    // Prevent changing value of spinbox with mouse wheel
    // This could happen while scrolling the scroll area
    // Also prevent getting focused by mouse wheel by setting StrongFocus
    const auto spinBoxes = findChildren<QAbstractSpinBox *>();
    for(QAbstractSpinBox *spin : spinBoxes)
    {
        spin->setFocusPolicy(Qt::StrongFocus);
        spin->installEventFilter(this);
    }

    const auto comboBoxes = findChildren<QComboBox *>();
    for(QComboBox *combo : comboBoxes)
    {
        combo->setFocusPolicy(Qt::StrongFocus);
        combo->installEventFilter(this);
    }
}

bool SimulationObjectOptionsWidget::eventFilter(QObject *watched, QEvent *ev)
{
    if(ev->type() == QEvent::Wheel)
    {
        // Allow reacting to mouse wheel only if already has focus
        if(QAbstractSpinBox *spin = qobject_cast<QAbstractSpinBox *>(watched))
        {
            if(!spin->hasFocus())
            {
                ev->ignore();
                return true;
            }
        }
        else if(QComboBox *combo = qobject_cast<QComboBox *>(watched))
        {
            if(!combo->hasFocus())
            {
                ev->ignore();
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, ev);
}

void SimulationObjectOptionsWidget::setEditingAllowed(bool value)
{
    // TODO: set read only on single fields instead of disabling full widget
    scrollArea->widget()->setEnabled(value);
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

void SimulationObjectOptionsWidget::onNodeClicked(const QModelIndex &idx)
{
    // Open new view if Shift is pressed, use existing otherwise
    const bool forceNew = QGuiApplication::keyboardModifiers()
            .testFlag(Qt::ShiftModifier);

    // Adjust zoom if Alt modifier is NOT pressed
    const bool adjustZoom = !QGuiApplication::keyboardModifiers()
            .testFlag(Qt::AltModifier);

    auto item = mNodesModel->itemAt(idx.row());
    if(!item)
        return;

    mViewMgr->ensureCircuitItemIsVisible(item, forceNew, adjustZoom);
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
