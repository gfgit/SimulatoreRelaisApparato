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

#include "../model/genericleverobject.h"
#include "../model/genericlevermodel.h"
#include "../model/leverpositionmodel.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>

GenericLeverOptionsWidget::GenericLeverOptionsWidget(GenericLeverModel *m,
                                                     GenericLeverObject *lever,
                                                     QWidget *parent)
    : QWidget{parent}
    , mModel(m)
    , mLever(lever)
{
    QFormLayout *lay = new QFormLayout(this);

    // Name
    mNameEdit = new QLineEdit;
    mNameEdit->setPlaceholderText(tr("Name"));
    mNameEdit->setText(mLever->name());
    lay->addRow(tr("Name:"), mNameEdit);

    normalEditPalette = mNameEdit->palette();

    // Spring return
    mHasSpringReturn = new QCheckBox(tr("Spring return to normal"));
    mHasSpringReturn->setChecked(mLever->hasSpringReturn());
    lay->addWidget(mHasSpringReturn);

    // Initial position and range
    mMinPosModel = new LeverPositionModel(mLever->positionDesc(), this);
    mMaxPosModel = new LeverPositionModel(mLever->positionDesc(), this);
    mInitPosModel = new LeverPositionModel(mLever->positionDesc(), this);

    mMinPosCombo = new QComboBox;
    mMinPosCombo->setModel(mMinPosModel);
    lay->addRow(tr("Minimum Position:"), mMinPosCombo);

    mMaxPosCombo = new QComboBox;
    mMaxPosCombo->setModel(mMaxPosModel);
    lay->addRow(tr("Maximum Position:"), mMaxPosCombo);

    mInitPosCombo = new QComboBox;
    mInitPosCombo->setModel(mInitPosModel);
    lay->addRow(tr("Initial Position:"), mInitPosCombo);

    connect(mNameEdit, &QLineEdit::editingFinished,
            this, &GenericLeverOptionsWidget::setLeverName);
    connect(mNameEdit, &QLineEdit::textEdited,
            this, &GenericLeverOptionsWidget::onNameTextEdited);

    connect(mHasSpringReturn, &QCheckBox::toggled,
            this, [this](bool val)
    {
        mLever->setHasSpringReturn(val);
    });

    connect(mLever, &GenericLeverObject::changed,
            this, &GenericLeverOptionsWidget::updatePositionRanges);

    updatePositionRanges();

    connect(mMinPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change min
        mLever->setAbsoluteRange(mMinPosModel->positionAt(idx),
                                 mLever->absoluteMax());
    });

    connect(mMaxPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change max
        mLever->setAbsoluteRange(mLever->absoluteMin(),
                                 mMaxPosModel->positionAt(idx));
    });

    connect(mInitPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change initial position
        mLever->setInitialPosition(mInitPosModel->positionAt(idx));
    });
}

void GenericLeverOptionsWidget::setLeverName()
{
    QString newName = mNameEdit->text().trimmed();
    if(!newName.isEmpty() && mModel->isNameAvailable(newName))
    {
        // Name is valid, set it
        mLever->setName(newName);
        return;
    }

    // Name is not valid, go back to old name
    mNameEdit->setText(mLever->name());
    setNameValid(true);
}

void GenericLeverOptionsWidget::onNameTextEdited()
{
    QString newName = mNameEdit->text().trimmed();

    bool valid = true;
    if(newName != mLever->name())
        valid = mModel->isNameAvailable(newName);

    setNameValid(valid);
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
        const int posIdx = mMinPosModel->rowForPosition(minPos);
        if(mMinPosCombo->currentIndex() != posIdx)
            mMinPosCombo->setCurrentIndex(posIdx);

        // Allow setting Max to at least Min position
        mMaxPosModel->setPositionRange(minPos,
                                       mLever->positionDesc().maxPosition());
    }

    if(updateMin || updateMax)
    {
        // Max positions could have changed index, update it
        const int maxPosIdx = mMaxPosModel->rowForPosition(maxPos);
        if(mMaxPosCombo->currentIndex() != maxPosIdx)
            mMaxPosCombo->setCurrentIndex(maxPosIdx);

        // Initial position must be in range min/max
        mInitPosModel->setPositionRange(minPos, maxPos);
    }

    const int initPosIdx = mInitPosModel->rowForPosition(mLever->initialPosition());
    if(initPosIdx != -1 && mInitPosCombo->currentIndex() != initPosIdx)
        mInitPosCombo->setCurrentIndex(initPosIdx);
}

void GenericLeverOptionsWidget::setNameValid(bool valid)
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
