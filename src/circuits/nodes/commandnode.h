/**
 * src/circuits/nodes/commandnode.h
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

#ifndef COMMANDNODE_H
#define COMMANDNODE_H

#include "abstractcircuitnode.h"
#include <QBasicTimer>

class AbstractSimulationObject;

struct EnumDesc;

class CommandNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit CommandNode(ModeManager *mgr, QObject *parent = nullptr);
    ~CommandNode();

    static constexpr QLatin1String NodeType = QLatin1String("command_node");
    QString nodeType() const override;

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences& items) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    AbstractSimulationObject *object() const;
    void setObject(AbstractSimulationObject *newObject);

    int delayMillis() const;
    void setDelayMillis(int newDelayMillis);

    int targetPosition() const;
    void setTargetPosition(int newTargetPosition);

    bool getObjectPosDesc(EnumDesc &descOut) const;

    QStringList supportedObjectTypes() const;

signals:
    void objectChanged(AbstractSimulationObject *obj);

protected:
    void timerEvent(QTimerEvent *ev) override;

    void performAction();

private:
    AbstractSimulationObject *mObject = nullptr;
    QBasicTimer mTimer;
    int mDelayMillis = 500;

    int mTargetPosition = 0;
};

#endif // COMMANDNODE_H

