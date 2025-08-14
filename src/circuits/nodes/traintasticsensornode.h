/**
 * src/circuits/nodes/traintasticsensornode.h
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

#ifndef TRAINTASTIC_SENSOR_NODE_H
#define TRAINTASTIC_SENSOR_NODE_H

#include "abstractdeviatornode.h"

class TraintasticSensorObj;

class TraintasticSensorNode : public AbstractDeviatorNode
{
    Q_OBJECT
public:
    explicit TraintasticSensorNode(ModeManager *mgr, QObject *parent = nullptr);
    ~TraintasticSensorNode();

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    static constexpr QLatin1String NodeType = QLatin1String("traintastic_sensor_contact");
    QString nodeType() const override;

    TraintasticSensorObj *sensor() const;
    void setSensor(TraintasticSensorObj *newButton);

    int targetState() const;
    void setTargetState(int newTargetState);

signals:
    void sensorChanged(TraintasticSensorObj *obj);

private:
    void refreshContactState();

private:
    friend class TraintasticSensorObj;

    TraintasticSensorObj *mSensor = nullptr;
    int mTargetState = 0;
};

#endif // TRAINTASTIC_SENSOR_NODE_H
