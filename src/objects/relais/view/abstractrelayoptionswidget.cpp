/**
 * src/objects/relais/view/abstractrelayoptionswidget.cpp
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

#include "abstractrelayoptionswidget.h"

#include "../model/abstractrelais.h"
#include "../model/relaismodel.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>

#include <QStringListModel>

AbstractRelayOptionsWidget::AbstractRelayOptionsWidget(RelaisModel *m,
                                                       AbstractRelais *relay,
                                                       QWidget *parent)
    : QWidget{parent}
    , mModel(m)
    , mRelay(relay)
{
    QFormLayout *lay = new QFormLayout(this);

    // Name
    mNameEdit = new QLineEdit;
    mNameEdit->setPlaceholderText(tr("Name"));
    mNameEdit->setText(mRelay->name());
    lay->addRow(tr("Name:"), mNameEdit);

    normalEditPalette = mNameEdit->palette();

    // Normally Up
    mNormallyUp = new QCheckBox(tr("Relay normally Up"));
    mNormallyUp->setChecked(mRelay->normallyUp());
    lay->addWidget(mNormallyUp);

    // Type
    mTypeCombo = new QComboBox;
    lay->addRow(tr("Type:"), mTypeCombo);

    QStringList typeList;
    typeList.reserve(int(AbstractRelais::Type::NTypes));
    for(int i = 0; i < int(AbstractRelais::Type::NTypes); i++)
    {
        typeList.append(AbstractRelais::getTypeName(AbstractRelais::Type(i)));
    }
    QStringListModel *typeModel = new QStringListModel(typeList, mTypeCombo);
    mTypeCombo->setModel(typeModel);
    mTypeCombo->setCurrentIndex(int(mRelay->type()));

    connect(mNameEdit, &QLineEdit::editingFinished,
            this, &AbstractRelayOptionsWidget::setRelaisName);
    connect(mNameEdit, &QLineEdit::textEdited,
            this, &AbstractRelayOptionsWidget::onNameTextEdited);

    connect(mNormallyUp, &QCheckBox::toggled,
            this, [this](bool val)
    {
        mRelay->setNormallyUp(val);
    });

    connect(mTypeCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        mRelay->setType(AbstractRelais::Type(idx));
    });
}

void AbstractRelayOptionsWidget::setRelaisName()
{
    QString newName = mNameEdit->text().trimmed();
    if(!newName.isEmpty() && mModel->isNameAvailable(newName))
    {
        // Name is valid, set it
        mRelay->setName(newName);
        return;
    }

    // Name is not valid, go back to old name
    mNameEdit->setText(mRelay->name());
    setNameValid(true);
}

void AbstractRelayOptionsWidget::onNameTextEdited()
{
    QString newName = mNameEdit->text().trimmed();

    bool valid = true;
    if(newName != mRelay->name())
        valid = mModel->isNameAvailable(newName);

    setNameValid(valid);
}

void AbstractRelayOptionsWidget::setNameValid(bool valid)
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
