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
#include <QSpinBox>

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

    // Custom Time
    // 50ms is minimum duration
    // We set 49 as real minimum and use it as "Default" value
    // which is relay default movement duration.
    mUpTimeSpin = new QSpinBox;
    mUpTimeSpin->setRange(49, 3000);
    mUpTimeSpin->setSpecialValueText(tr("Default"));
    mUpTimeSpin->setSuffix(tr(" ms"));
    lay->addRow(tr("Up duration:"), mUpTimeSpin);

    mDownTimeSpin = new QSpinBox;
    mDownTimeSpin->setRange(49, 3000);
    mDownTimeSpin->setSpecialValueText(tr("Default"));
    mDownTimeSpin->setSuffix(tr(" ms"));
    lay->addRow(tr("Down duration:"), mDownTimeSpin);

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

    connect(mRelay, &AbstractRelais::typeChanged,
            this, [this]()
    {
        mTypeCombo->setCurrentIndex(int(mRelay->relaisType()));
    });

    connect(mRelay, &AbstractRelais::settingsChanged,
            this, [this]()
    {
        updateDurations();
    });

    connect(mUpTimeSpin, &QSpinBox::valueChanged,
            this, &AbstractRelayOptionsWidget::setNewDurations);
    connect(mDownTimeSpin, &QSpinBox::valueChanged,
            this, &AbstractRelayOptionsWidget::setNewDurations);

    updateDurations();
}

void AbstractRelayOptionsWidget::updateDurations()
{
    const quint32 upDurationMS = mRelay->durationUp();
    if(upDurationMS == 0) // Default
        mUpTimeSpin->setValue(mUpTimeSpin->minimum());
    else
        mUpTimeSpin->setValue(int(upDurationMS));

    const quint32 downDurationMS = mRelay->durationDown();
    if(downDurationMS == 0) // Default
        mDownTimeSpin->setValue(mDownTimeSpin->minimum());
    else
        mDownTimeSpin->setValue(int(downDurationMS));
}

void AbstractRelayOptionsWidget::setNewDurations()
{
    int upDurationMS = mUpTimeSpin->value();
    if(upDurationMS == mUpTimeSpin->minimum())
        upDurationMS = 0; // Default
    mRelay->setDurationUp(quint32(upDurationMS));

    int downDurationMS = mDownTimeSpin->value();
    if(downDurationMS == mDownTimeSpin->minimum())
        downDurationMS = 0; // Default
    mRelay->setDurationDown(quint32(downDurationMS));
}
