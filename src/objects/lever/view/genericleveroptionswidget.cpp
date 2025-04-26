/**
 * src/objects/lever/view/genericleveroptionswidget.cpp
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

#include "genericleveroptionswidget.h"

#include "../../abstractsimulationobject.h"
#include "../../interfaces/leverinterface.h"

#include "../../../utils/enumvaluesmodel.h"

#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>

GenericLeverOptionsWidget::GenericLeverOptionsWidget(LeverInterface *lever,
                                                     QWidget *parent)
    : QWidget{parent}
    , mLever(lever)
{
    QFormLayout *lay = new QFormLayout(this);

    // Spring return
    mHasSpringReturnMin = new QCheckBox(tr("Spring return to normal if before default position"));
    mHasSpringReturnMin->setChecked(mLever->hasSpringReturnMin());
    mHasSpringReturnMin->setToolTip(tr("When released lever will return"
                                    " to its normal position."));
    lay->addWidget(mHasSpringReturnMin);
    lay->setRowVisible(mHasSpringReturnMin, mLever->canChangeSpring());

    mHasSpringReturnMax = new QCheckBox(tr("Spring return to normal if after default position"));
    mHasSpringReturnMax->setChecked(mLever->hasSpringReturnMax());
    mHasSpringReturnMax->setToolTip(mHasSpringReturnMin->toolTip());
    lay->addWidget(mHasSpringReturnMax);
    lay->setRowVisible(mHasSpringReturnMax, mLever->canChangeSpring());

    // Normal position and range
    mMinPosModel = new EnumValuesModel(this);
    mMaxPosModel = new EnumValuesModel(this);
    mNormalPosModel = new EnumValuesModel(this);

    mMinPosCombo = new QComboBox;
    mMinPosCombo->setModel(mMinPosModel);
    lay->addRow(tr("Minimum Position:"), mMinPosCombo);

    mMaxPosCombo = new QComboBox;
    mMaxPosCombo->setModel(mMaxPosModel);
    lay->addRow(tr("Maximum Position:"), mMaxPosCombo);

    lay->setRowVisible(mMinPosCombo, mLever->canChangeRange());
    lay->setRowVisible(mMaxPosCombo, mLever->canChangeRange());

    mNormalPosCombo = new QComboBox;
    mNormalPosCombo->setModel(mNormalPosModel);
    mNormalPosCombo->setToolTip(tr("On program start,"
                                   " lever will be in this position."));
    lay->addRow(tr("Normal Position:"), mNormalPosCombo);

    connect(mHasSpringReturnMin, &QCheckBox::toggled,
            this, [this](bool val)
    {
        mLever->setHasSpringReturnMin(val);
    });

    connect(mHasSpringReturnMax, &QCheckBox::toggled,
            this, [this](bool val)
    {
        mLever->setHasSpringReturnMax(val);
    });

    connect(mLever->object(), &AbstractSimulationObject::settingsChanged,
            this, &GenericLeverOptionsWidget::updatePositionRanges);

    updatePositionDesc();

    connect(mMinPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change min
        mLever->setAbsoluteRange(mMinPosModel->valueAt(idx),
                                 mLever->absoluteMax());
    });

    connect(mMaxPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change max
        mLever->setAbsoluteRange(mLever->absoluteMin(),
                                 mMaxPosModel->valueAt(idx));
    });

    connect(mNormalPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change normal position
        mLever->setNormalPosition(mNormalPosModel->valueAt(idx));
    });

    connect(mLever->object(), &AbstractSimulationObject::interfacePropertyChanged,
            this, &GenericLeverOptionsWidget::onInterfacePropertyChanged);
}

void GenericLeverOptionsWidget::updatePositionDesc()
{
    mMinPosModel->setEnumDescFull(mLever->positionDesc(), true);
    mMaxPosModel->setEnumDescFull(mLever->positionDesc(), true);
    mNormalPosModel->setEnumDescFull(mLever->positionDesc(), true);

    updatePositionRanges();
}

void GenericLeverOptionsWidget::updatePositionRanges()
{
    int min = mLever->absoluteMin();
    int max = mLever->absoluteMax();

    bool updateMin = false;
    bool updateMax = false;

    if(min != minPos)
    {
        minPos = min;
        updateMin = true;
    }

    if(max != maxPos)
    {
        maxPos = max;
        updateMax = true;
    }

    if(updateMin)
    {
        const int posIdx = mMinPosModel->rowForValue(minPos);
        if(mMinPosCombo->currentIndex() != posIdx)
            mMinPosCombo->setCurrentIndex(posIdx);

        // Allow setting Max to at least Min position
        mMaxPosModel->setEnumDescRange(mMaxPosModel->enumDesc(), true,
                                       minPos,
                                       mLever->positionDesc().maxValue);
    }

    if(updateMin || updateMax)
    {
        // Max positions could have changed index, update it
        const int maxPosIdx = mMaxPosModel->rowForValue(maxPos);
        if(mMaxPosCombo->currentIndex() != maxPosIdx)
            mMaxPosCombo->setCurrentIndex(maxPosIdx);

        // Normal position must be in range min/max
        mNormalPosModel->setEnumDescRange(mNormalPosModel->enumDesc(), true,
                                          minPos, maxPos);
    }

    const int normalPosIdx = mNormalPosModel->rowForValue(mLever->normalPosition());
    if(normalPosIdx != -1 && mNormalPosCombo->currentIndex() != normalPosIdx)
        mNormalPosCombo->setCurrentIndex(normalPosIdx);

    const int defaultPos = mLever->positionDesc().defaultValue;
    mHasSpringReturnMin->setEnabled(minPos < defaultPos && defaultPos <= maxPos);
    mHasSpringReturnMax->setEnabled(minPos <= defaultPos && defaultPos < maxPos);
}

void GenericLeverOptionsWidget::onInterfacePropertyChanged(const QString &ifaceName,
                                                           const QString &propName,
                                                           const QVariant &value)
{
    if(ifaceName == LeverInterface::IfaceType)
    {
        if(!mLever)
            return;

        if(propName == LeverInterface::PosDescPropName)
        {
            updatePositionDesc();
        }
    }
}
