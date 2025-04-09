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

#include "../../../utils/enumvaluesmodel.h"

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

    mCodeComboModel = new EnumValuesModel(this);
    mCodeComboModel->setEnumDescFull(SignalAspectCode_getDesc(), false);

    mCodeCombo = new QComboBox;
    mCodeCombo->setModel(mCodeComboModel);
    lay->addRow(tr("Code:"), mCodeCombo);

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

        const bool isEncoder = mRelay->relaisType() == AbstractRelais::RelaisType::Encoder;
        const bool isDecoder = mRelay->relaisType() == AbstractRelais::RelaisType::Decoder;
        if(isEncoder)
        {
            mCodeComboModel->setEnumDescRange(SignalAspectCode_getDesc(), false,
                                              int(SignalAspectCode::Code75),
                                              int(SignalAspectCode::Code270));
        }
        else if(isDecoder)
        {
            mCodeComboModel->setEnumDescFull(SignalAspectCode_getDesc(), false);
        }

        mCodeCombo->setCurrentIndex(mCodeComboModel->rowForValue(int(mRelay->getExpectedCode())));
    });

    connect(mRelay, &AbstractRelais::settingsChanged,
            this, [this]()
    {
        updateDurations();
    });

    connect(mUpTimeSpin, &QSpinBox::editingFinished,
            this, &AbstractRelayOptionsWidget::setNewDurations);
    connect(mDownTimeSpin, &QSpinBox::editingFinished,
            this, &AbstractRelayOptionsWidget::setNewDurations);
    connect(mCodeCombo, &QComboBox::activated,
            this, &AbstractRelayOptionsWidget::setNewDurations);

    updateDurations();
}

void AbstractRelayOptionsWidget::updateDurations()
{
    if(mRelay->relaisType() == AbstractRelais::RelaisType::Blinker)
    {
        mDownTimeSpin->setSpecialValueText(tr("Symmetric"));
    }
    else
    {
        mDownTimeSpin->setSpecialValueText(tr("Default"));
    }

    const bool isEncoder = mRelay->relaisType() == AbstractRelais::RelaisType::Encoder;
    const bool isDecoder = mRelay->relaisType() == AbstractRelais::RelaisType::Decoder;
    const bool isRepeater = mRelay->relaisType() == AbstractRelais::RelaisType::CodeRepeater;

    QFormLayout *lay = static_cast<QFormLayout *>(layout());
    lay->setRowVisible(mUpTimeSpin,   !isEncoder && !isRepeater);
    lay->setRowVisible(mDownTimeSpin, !isEncoder && !isRepeater);
    lay->setRowVisible(mCodeCombo, isEncoder || isDecoder);

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

    if(isEncoder || isDecoder)
    {
        mCodeCombo->setCurrentIndex(mCodeComboModel->rowForValue(int(mRelay->getExpectedCode())));
    }
}

void AbstractRelayOptionsWidget::setNewDurations()
{
    const bool isEncoder = mRelay->relaisType() == AbstractRelais::RelaisType::Encoder ||
            mRelay->relaisType() ==  AbstractRelais::RelaisType::Decoder;

    if(isEncoder)
    {
        int curIdx = mCodeCombo->currentIndex();
        int val = mCodeComboModel->valueAt(mCodeCombo->currentIndex());

        SignalAspectCode code = SignalAspectCode(val);
        if(curIdx < 0)
            code = SignalAspectCode::CodeAbsent;

        mRelay->setExpectedCode(code);
        return;
    }

    int upDurationMS = mUpTimeSpin->value();
    if(upDurationMS == mUpTimeSpin->minimum())
        upDurationMS = 0; // Default
    mRelay->setDurationUp(quint32(upDurationMS));

    int downDurationMS = mDownTimeSpin->value();
    if(downDurationMS == mDownTimeSpin->minimum())
        downDurationMS = 0; // Default
    mRelay->setDurationDown(quint32(downDurationMS));
}
