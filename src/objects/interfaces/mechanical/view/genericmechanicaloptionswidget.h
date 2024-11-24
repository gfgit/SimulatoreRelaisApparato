/**
 * src/objects/interfaces/mechanical/view/genericmechanicaloptionswidget.h
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

#ifndef GENERIC_MECHANICAL_OPTIONSWIDGET_H
#define GENERIC_MECHANICAL_OPTIONSWIDGET_H

#include <QWidget>

class QComboBox;

class MechanicalInterface;

class EnumValuesModel;

class GenericMechanicalOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    GenericMechanicalOptionsWidget(MechanicalInterface *lever,
                                   QWidget *parent = nullptr);

private slots:
    void updatePositionRanges();

private:
    MechanicalInterface *mMechanicalIface = nullptr;

    QComboBox *mMinPosCombo = nullptr;
    QComboBox *mMaxPosCombo = nullptr;
    QComboBox *mNormalPosCombo = nullptr;

    EnumValuesModel *mMinPosModel = nullptr;
    EnumValuesModel *mMaxPosModel = nullptr;
    EnumValuesModel *mNormalPosModel = nullptr;

    int minPos = -1;
    int maxPos = -1;
};

#endif // GENERIC_MECHANICAL_OPTIONSWIDGET_H
