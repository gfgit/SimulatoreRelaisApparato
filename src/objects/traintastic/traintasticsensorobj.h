/**
 * src/objects/traintastic/traintasticsensorobj.h
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

#ifndef TRAINTASTICSENSOROBJ_H
#define TRAINTASTICSENSOROBJ_H

#include "../abstractsimulationobject.h"
#include <QVector>

class TraintasticSensorNode;

class TraintasticTurnoutObj;

class TraintasticSensorObj : public AbstractSimulationObject
{
    Q_OBJECT
public:
    enum SensorType
    {
        Generic = 0,
        TurnoutFeedback = 1,
        Spawn = 2
    };

    static constexpr int InvalidChannel = -1;
    static constexpr int InvalidAddress = -1;

    explicit TraintasticSensorObj(AbstractSimulationObjectModel *m);
    ~TraintasticSensorObj();

    static constexpr QLatin1String Type = QLatin1String("traintastic_sensor");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    inline int channel() const { return mChannel; }
    inline int address() const { return mAddress; }
    inline int state() const { return mState; }

    inline SensorType sensorType() const { return mSensorType; }

    bool setChannel(int newChannel);
    bool setAddress(int newAddress);
    void setSensorType(SensorType newSensorType);

    inline int defaultOffState() const { return mDefaultOffState; }

    void setDefaultOffState(int newDefaultOffState);

    TraintasticTurnoutObj *shuntTurnout() const;
    bool setShuntTurnout(TraintasticTurnoutObj *newShuntTurnout);

private:
    friend class TraintasticSimManager;
    friend class TraintasticTurnoutObj;
    void setState(int newState);

    friend class TraintasticSensorNode;
    void addContactNode(TraintasticSensorNode *c);
    void removeContactNode(TraintasticSensorNode *c);

    void onTraintasticDisconnected();

private:
    QVector<TraintasticSensorNode *> mContactNodes;

    TraintasticTurnoutObj *mShuntTurnout = nullptr;

    int mChannel = 0; // Default to valid channel
    int mAddress = InvalidAddress;
    int mState = 0;
    int mDefaultOffState = 1;
    SensorType mSensorType = SensorType::Generic;
};

#endif // TRAINTASTICSENSOROBJ_H
