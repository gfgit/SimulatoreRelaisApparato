/**
 * src/circuits/nodes/traintasticaxlecounternode.h
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

#ifndef TRAINTASTIC_AXLE_COUNTER_NODE_H
#define TRAINTASTIC_AXLE_COUNTER_NODE_H

#include "abstractcircuitnode.h"

class TraintasticAxleCounterObj;

class TraintasticAxleCounterNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    enum Contacts
    {
        FreeTrackOut = 0,
        ResetIn = 1,
        OccupiedTrackOut = 2,
        PowerIn = 3
    };

    explicit TraintasticAxleCounterNode(ModeManager *mgr, QObject *parent = nullptr);
    ~TraintasticAxleCounterNode();

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir = false) override;

    static constexpr QLatin1String NodeType = QLatin1String("traintastic_axle_counter_node");
    QString nodeType() const override;

    bool loadFromJSON(const QJsonObject &obj) override;
    void saveToJSON(QJsonObject &obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    TraintasticAxleCounterObj *axleCounter() const;
    bool setAxleCounter(TraintasticAxleCounterObj *newAxleCounter, bool steal = false);

    void updateState(bool circuitChange);

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items) override;
    void partialRemoveCircuit(ElectricCircuit *circuit,
                                      const NodeOccurences &items) override;

private:
    void updateInputState();

private:
    TraintasticAxleCounterObj *mAxleCounter = nullptr;
};

#endif // TRAINTASTIC_AXLE_COUNTER_NODE_H
