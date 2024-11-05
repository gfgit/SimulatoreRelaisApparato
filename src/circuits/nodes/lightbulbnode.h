/**
 * src/circuits/nodes/lightbulbnode.h
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

#ifndef LIGHTBULBNODE_H
#define LIGHTBULBNODE_H

#include "abstractcircuitnode.h"

class LightBulbNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit LightBulbNode(ModeManager *mgr, QObject *parent = nullptr);
    ~LightBulbNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    static constexpr QLatin1String NodeType = QLatin1String("light_bulb");
    QString nodeType() const override;
};

#endif // LIGHTBULBNODE_H