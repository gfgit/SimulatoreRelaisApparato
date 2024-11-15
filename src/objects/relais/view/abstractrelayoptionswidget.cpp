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

#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>

#include <QStringListModel>

AbstractRelayOptionsWidget::AbstractRelayOptionsWidget(AbstractRelais *relay,
                                                       QWidget *parent)
    : QWidget{parent}
    , mRelay(relay)
{
    QFormLayout *lay = new QFormLayout(this);

    // Normally Up
    mNormallyUp = new QCheckBox(tr("Relay normally Up"));
    mNormallyUp->setChecked(mRelay->normallyUp());
    lay->addWidget(mNormallyUp);

    // Type
    mTypeCombo = new QComboBox;
    lay->addRow(tr("Type:"), mTypeCombo);

    QStringList typeList;
    typeList.reserve(int(AbstractRelais::RelaisType::NTypes));
    for(int i = 0; i < int(AbstractRelais::RelaisType::NTypes); i++)
    {
        typeList.append(AbstractRelais::getRelaisTypeName(AbstractRelais::RelaisType(i)));
    }
    QStringListModel *typeModel = new QStringListModel(typeList, mTypeCombo);
    mTypeCombo->setModel(typeModel);
    mTypeCombo->setCurrentIndex(int(mRelay->relaisType()));

    connect(mNormallyUp, &QCheckBox::toggled,
            this, [this](bool val)
    {
        mRelay->setNormallyUp(val);
    });

    connect(mTypeCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        mRelay->setRelaisType(AbstractRelais::RelaisType(idx));
    });
}
