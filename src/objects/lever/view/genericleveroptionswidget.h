/**
 * src/objects/lever/view/genericleveroptionswidget.h
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

#ifndef GENERICLEVEROPTIONSWIDGET_H
#define GENERICLEVEROPTIONSWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;
class QComboBox;

class GenericLeverObject;
class GenericLeverModel;

class LeverPositionModel;

class GenericLeverOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    GenericLeverOptionsWidget(GenericLeverModel *m,
                              GenericLeverObject *lever,
                              QWidget *parent = nullptr);

private slots:
    void setLeverName();
    void onNameTextEdited();
    void updatePositionRanges();

private:
    void setNameValid(bool valid);

private:
    GenericLeverModel *mModel = nullptr;
    GenericLeverObject *mLever = nullptr;

    QLineEdit *mNameEdit = nullptr;
    QCheckBox *mHasSpringReturn = nullptr;

    QComboBox *mMinPosCombo = nullptr;
    QComboBox *mMaxPosCombo = nullptr;
    QComboBox *mInitPosCombo = nullptr;

    LeverPositionModel *mMinPosModel = nullptr;
    LeverPositionModel *mMaxPosModel = nullptr;
    LeverPositionModel *mInitPosModel = nullptr;

    QPalette normalEditPalette;

    int minPos = -1;
    int maxPos = -1;
};

#endif // GENERICLEVEROPTIONSWIDGET_H
