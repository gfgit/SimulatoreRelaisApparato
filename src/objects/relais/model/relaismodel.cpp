/**
 * src/objects/relais/model/relaismodel.cpp
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

#include "relaismodel.h"

#include "abstractrelais.h"

#include "../../../views/modemanager.h"

#include <QColor>

#include <QJsonObject>
#include <QJsonArray>

RelaisModel::RelaisModel(ModeManager *mgr, QObject *parent)
    : AbstractSimulationObjectModel(mgr, AbstractRelais::Type, parent)
{
}

QVariant RelaisModel::data(const QModelIndex &idx, int role) const
{
    const AbstractRelais *relay = static_cast<AbstractRelais *>(objectAt(idx.row()));

    if(relay && idx.column() == 0 && role == Qt::DecorationRole)
    {
        // Show a little colored square based on relay state
        QColor color = Qt::black;
        switch (relay->state())
        {
        case AbstractRelais::State::Up:
            color = Qt::red;
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color.setRgb(120, 210, 255); // Light blue
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }

        return color;
    }

    return AbstractSimulationObjectModel::data(idx, role);
}
