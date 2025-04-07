/**
 * src/circuits/nodes/powersourcenode.h
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

#ifndef POWERSOURCENODE_H
#define POWERSOURCENODE_H

#include "abstractcircuitnode.h"

class PowerSourceNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit PowerSourceNode(ModeManager *mgr, QObject *parent = nullptr);

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir = false) override;

    static constexpr QLatin1String NodeType = QLatin1String("power_source");
    QString nodeType() const override;

    bool isSourceNode(bool onlyCurrentState) const override;

    bool isSourceEnabled() const override;
    void setSourceEnabled(bool newEnabled) override;

signals:
    void enabledChanged(bool val);

private:
    bool enabled = false;
};

#endif // POWERSOURCENODE_H
