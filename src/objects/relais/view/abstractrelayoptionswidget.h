/**
 * src/objects/relais/view/abstractrelayoptionswidget.h
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

#ifndef ABSTRACT_RELAY_OPTIONSWIDGET_H
#define ABSTRACT_RELAY_OPTIONSWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;
class QComboBox;

class AbstractRelais;
class RelaisModel;

class AbstractRelayOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    AbstractRelayOptionsWidget(RelaisModel *m,
                              AbstractRelais *lever,
                              QWidget *parent = nullptr);

private slots:
    void setRelaisName();
    void onNameTextEdited();

private:
    void setNameValid(bool valid);

private:
    RelaisModel *mModel = nullptr;
    AbstractRelais *mRelay = nullptr;

    QLineEdit *mNameEdit = nullptr;
    QCheckBox *mNormallyUp = nullptr;

    QComboBox *mTypeCombo = nullptr;

    QPalette normalEditPalette;
};

#endif // ABSTRACT_RELAY_OPTIONSWIDGET_H
