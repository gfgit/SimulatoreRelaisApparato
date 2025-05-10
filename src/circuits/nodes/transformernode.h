/**
 * src/circuits/nodes/transformernode.h
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#ifndef TRANSFORMERNODE_H
#define TRANSFORMERNODE_H

#include "abstractcircuitnode.h"

class TransformerNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit TransformerNode(ModeManager *mgr, QObject *parent = nullptr);

    bool event(QEvent *e) override;

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items) override;
    void partialRemoveCircuit(ElectricCircuit *circuit,
                                      const NodeOccurences &items) override;

    static constexpr QLatin1String NodeType = QLatin1String("transformer");
    QString nodeType() const override;

    bool isSourceNode(bool onlyCurrentState, int nodeContact = NodeItem::InvalidContact) const override;

    bool isSourceEnabled() const override;
    void setSourceEnabled(bool newEnabled) override;

private:
    void updateSourceState();

private:
    bool enabled = false;
    bool reallyEnabled = false;
};

#endif // TRANSFORMERNODE_H
