/**
 * src/objects/relais/view/relaylineedit.cpp
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

#include "relaylineedit.h"

#include <QCompleter>
#include <QAbstractProxyModel>

#include "../model/abstractrelais.h"
#include "../model/relaismodel.h"

RelayLineEdit::RelayLineEdit(RelaisModel *m, QWidget *parent)
    : QLineEdit(parent)
    , mRelaisModel(m)
{
    QCompleter *c = new QCompleter;
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setFilterMode(Qt::MatchContains);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->setModel(mRelaisModel);
    setCompleter(c);

    connect(c, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        setRelais(mRelaisModel->relayAt(idx.row()));
    });

    connect(c, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this, c](const QModelIndex& idx)
    {
        QAbstractProxyModel *m = static_cast<QAbstractProxyModel *>(c->completionModel());
        const QModelIndex sourceIdx = m->mapToSource(idx);

        setRelais(mRelaisModel->relayAt(sourceIdx.row()));
    });
}

void RelayLineEdit::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;
    mRelais = newRelais;

    if(text() != mRelais->name())
        setText(mRelais->name());

    emit relayChanged(mRelais);
}
