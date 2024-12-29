/**
 * src/objects/screen_relais/view/screenrelaisoptionswidget.cpp
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

#include "screenrelaisoptionswidget.h"

#include "../model/screenrelais.h"

#include <QFormLayout>
#include <QComboBox>

#include "../../../utils/enumvaluesmodel.h"

ScreenRelaisOptionsWidget::ScreenRelaisOptionsWidget(ScreenRelais *relay,
                                                     QWidget *parent)
    : QWidget{parent}
    , mRelay(relay)
{
    QFormLayout *lay = new QFormLayout(this);

    // Glass Colors
    EnumValuesModel *colorModel = new EnumValuesModel(this);
    colorModel->setEnumDescFull(ScreenRelais::getGlassColorDesc(), false);

    QComboBox *colorCombo0 = new QComboBox;
    colorCombo0->setModel(colorModel);

    QComboBox *colorCombo1 = new QComboBox;
    colorCombo1->setModel(colorModel);

    QComboBox *colorCombo2 = new QComboBox;
    colorCombo2->setModel(colorModel);

    QObject::connect(colorCombo0, &QComboBox::activated,
                     this, [this, colorModel](int idx)
    {
        mRelay->setColorAt(0, ScreenRelais::GlassColor(colorModel->valueAt(idx)));
    });

    lay->addRow(tr("Color 0:"), colorCombo0);

    QObject::connect(colorCombo1, &QComboBox::activated,
                     this, [this, colorModel](int idx)
    {
        mRelay->setColorAt(1, ScreenRelais::GlassColor(colorModel->valueAt(idx)));
    });

    lay->addRow(tr("Color 1:"), colorCombo1);

    QObject::connect(colorCombo2, &QComboBox::activated,
                     this, [this, colorModel](int idx)
    {
        mRelay->setColorAt(2, ScreenRelais::GlassColor(colorModel->valueAt(idx)));
    });

    lay->addRow(tr("Color 2:"), colorCombo2);

    auto updateSettings = [this, colorCombo0, colorCombo1, colorCombo2, colorModel]()
    {
        colorCombo0->setCurrentIndex(colorModel->rowForValue(int(mRelay->getColorAt(0))));
        colorCombo1->setCurrentIndex(colorModel->rowForValue(int(mRelay->getColorAt(1))));
        colorCombo2->setCurrentIndex(colorModel->rowForValue(int(mRelay->getColorAt(2))));
    };

    QObject::connect(mRelay, &ScreenRelais::settingsChanged,
                     this, updateSettings);

    updateSettings();
}
