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

class TraintasticSensorObj : public AbstractSimulationObject
{
    Q_OBJECT
public:
    enum SensorType
    {
        Generic = 0,
        TurnoutFeedback = 1
    };

    explicit TraintasticSensorObj(AbstractSimulationObjectModel *m);
    ~TraintasticSensorObj();

    static constexpr QLatin1String Type = QLatin1String("traintastic_sensor");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    inline int channel() const { return mChannel; }
    inline int address() const { return mAddress; }
    inline int state() const { return mState; }

    inline SensorType sensorType() const { return mSensorType; }

    bool setChannel(int newChannel);
    bool setAddress(int newAddress);
    void setSensorType(SensorType newSensorType);

private:
    friend class TraintasticSimManager;
    void setState(int newState);

private:
    int mChannel = -1;
    int mAddress = -1;
    int mState = 0;
    SensorType mSensorType = SensorType::Generic;
};

#endif // TRAINTASTICSENSOROBJ_H
