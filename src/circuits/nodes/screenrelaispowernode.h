/**
 * src/circuits/nodes/screenrelaispowernode.h
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

#ifndef SCREEN_RELAIS_POWER_NODE_H
#define SCREEN_RELAIS_POWER_NODE_H

#include "abstractcircuitnode.h"

class ScreenRelais;

class ScreenRelaisPowerNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit ScreenRelaisPowerNode(ModeManager *mgr, QObject *parent = nullptr);
    ~ScreenRelaisPowerNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences& items) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("screen_relais_power");
    QString nodeType() const override;

    ScreenRelais *screenRelais() const;
    void setScreenRelais(ScreenRelais *newRelais);

signals:
    void relayChanged(ScreenRelais *r);

private slots:
    void onScreenTypeChanged();

private:
    void updateScreenPower();

private:
    ScreenRelais *mScreenRelais = nullptr;

    bool mCircuitIsInverted = false;
};

#endif // SCREEN_RELAIS_POWER_NODE_H
