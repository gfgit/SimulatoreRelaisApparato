/**
 * src/objects/acei_lever/view/aceileverlineedit.cpp
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

#include "aceileverlineedit.h"

#include <QCompleter>


#include "../model/aceileverobject.h"
#include "../model/aceilevermodel.h"

ACEILeverLineEdit::ACEILeverLineEdit(ACEILeverModel *m, QWidget *parent)
    : QLineEdit(parent)
    , mACEILeverModel(m)
{
    QCompleter *c = new QCompleter;
    c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->setModel(mACEILeverModel);
    setCompleter(c);

    connect(c, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        setLever(mACEILeverModel->leverAt(idx.row()));
    });
}

void ACEILeverLineEdit::setLever(ACEILeverObject *newLever)
{
    if(mLever == newLever)
        return;
    mLever = newLever;

    if(text() != mLever->name())
        setText(mLever->name());

    emit leverChanged(mLever);
}
