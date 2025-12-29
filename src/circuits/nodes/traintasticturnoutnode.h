/**
 * src/circuits/nodes/traintasticturnoutnode.h
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

#ifndef TRAINTASTIC_TURNOUT_NODE_H
#define TRAINTASTIC_TURNOUT_NODE_H

#include "abstractcircuitnode.h"

class TraintasticTurnoutObj;
class TraintasticSpawnObj;
class TraintasticAuxSignalObject;

class TraintasticTurnoutNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit TraintasticTurnoutNode(ModeManager *mgr, QObject *parent = nullptr);
    ~TraintasticTurnoutNode();

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir) override;
    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    static constexpr QLatin1String NodeType = QLatin1String("traintastic_turnout_node");
    QString nodeType() const override;

    TraintasticTurnoutObj *turnout() const;
    bool setTurnout(TraintasticTurnoutObj *newTurnout, bool steal = false);

    TraintasticSpawnObj *spawn() const;
    bool setSpawn(TraintasticSpawnObj *newSpawn, bool steal = false);

    TraintasticAuxSignalObject *auxSignal() const;
    void setAuxSignal(TraintasticAuxSignalObject *newAuxSignal);

signals:
    void turnoutChanged(TraintasticTurnoutObj *obj);
    void spawnChanged(TraintasticSpawnObj *obj);
    void auxSignalChanged(TraintasticAuxSignalObject *obj);

private slots:
    void onAuxSignalsDestroyed(QObject *obj);

private:
    void updateState();

private:
    friend class TraintasticTurnoutObj;
    TraintasticTurnoutObj *mTurnout = nullptr;
    TraintasticSpawnObj *mSpawn = nullptr;
    TraintasticAuxSignalObject *mAuxSignal = nullptr;
};

#endif // TRAINTASTIC_TURNOUT_NODE_H
