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
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

#include <QStringListModel>

ScreenRelaisOptionsWidget::ScreenRelaisOptionsWidget(ScreenRelais *relay,
                                                     QWidget *parent)
    : QWidget{parent}
    , mRelay(relay)
{
    QFormLayout *lay = new QFormLayout(this);


}
