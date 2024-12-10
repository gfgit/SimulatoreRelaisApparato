/**
 * src/circuits/nodes/diodecircuitnode.h
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

#ifndef DIODECIRCUITNODE_H
#define DIODECIRCUITNODE_H

#include "abstractcircuitnode.h"

class DiodeCircuitNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit DiodeCircuitNode(ModeManager *mgr, QObject *parent = nullptr);

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir) override;

    virtual void addCircuit(ElectricCircuit *circuit) override;
    void partialRemoveCircuit(ElectricCircuit *circuit, const NodeOccurences &items) override;

    static constexpr QLatin1String NodeType = QLatin1String("diode_node");
    QString nodeType() const override;

    inline bool hasPassingCircuits(CircuitType type) const
    {
        if(type == CircuitType::Closed)
            return mPassingClosedCircuitsCount > 0;
        return mPassingOpenCircuitsCount > 0;
    }

    AnyCircuitType hasAnyPassingCircuits() const
    {
        if(hasPassingCircuits(CircuitType::Closed))
            return AnyCircuitType::Closed;
        if(hasPassingCircuits(CircuitType::Open))
            return AnyCircuitType::Open;
        return AnyCircuitType::None;
    }

private:
    int mPassingClosedCircuitsCount = 0;
    int mPassingOpenCircuitsCount = 0;
};

#endif // DIODECIRCUITNODE_H
