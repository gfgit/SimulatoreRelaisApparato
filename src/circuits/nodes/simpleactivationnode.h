/**
 * src/circuits/nodes/simpleactivationnode.h
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

#ifndef SIMPLEACTIVATIONNODE_H
#define SIMPLEACTIVATIONNODE_H

#include "abstractcircuitnode.h"

class AbstractSimpleActivableObject;

class SimpleActivationNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit SimpleActivationNode(ModeManager *mgr, QObject *parent = nullptr);
    ~SimpleActivationNode();

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences& items) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    AbstractSimpleActivableObject *object() const;
    void setObject(AbstractSimpleActivableObject *newObject);

    virtual QString allowedObjectType() const = 0;

signals:
    void objectChanged(AbstractSimpleActivableObject *obj);

private:
    AbstractSimpleActivableObject *mObject = nullptr;
};

#endif // SIMPLEACTIVATIONNODE_H
