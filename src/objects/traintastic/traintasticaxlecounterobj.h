/**
 * src/objects/traintastic/traintasticaxlecounterobj.h
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

#ifndef TRAINTASTIC_AXLE_COUNTER_OBJECT_H
#define TRAINTASTIC_AXLE_COUNTER_OBJECT_H

#include "../abstractsimulationobject.h"

#include <QBasicTimer>

class TraintasticAxleCounterNode;

class TraintasticAxleCounterObj : public AbstractSimulationObject
{
    Q_OBJECT
public:

    // Reset must be triggered for at least 3 seconds and less than 10 seconds
    // Otherwise it will go in OccupiedAtStart state
    enum State
    {
        OccupiedAtStart = 0,
        ResetPre,
        Reset,
        ResetPost,
        Free,
        Occupied
    };

    static constexpr int InvalidChannel = -1;
    static constexpr int InvalidAddress = -1;

    explicit TraintasticAxleCounterObj(AbstractSimulationObjectModel *m);
    ~TraintasticAxleCounterObj();

    static constexpr QLatin1String Type = QLatin1String("traintastic_axle_counter");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    inline int channel(bool first) const { return mSensors[first ? 0 : 1].channel; }
    inline int address(bool first) const { return mSensors[first ? 0 : 1].address; }
    inline bool invertCount(bool first) const { return mSensors[first ? 0 : 1].invertCount; }

    inline State state() const { return mState; }
    inline int32_t axleCount() const { return mAxleCount; }

    void setChannel(int newChannel, bool first);
    void setAddress(int newAddress, bool first);
    void setInvertCount(bool invert, bool first);

    inline bool hasContactNode() const { return mContactNode; }

    inline bool isResetting() const
    {
        switch (mState)
        {
        case State::ResetPre:
        case State::Reset:
        case State::ResetPost:
            return true;
        break;
        default:
        break;
        }
        return false;
    }

    void axleCounterEvent(int32_t axleCountDiff, bool firstSensor);

    QString getStateName() const;

protected:
    void timerEvent(QTimerEvent *e) override;

private:
    void setState(State newState);

    friend class TraintasticAxleCounterNode;
    void setContactNode(TraintasticAxleCounterNode *c);
    void triggerReset(bool val);
    void setHasPower(bool val);

private:
    TraintasticAxleCounterNode *mContactNode = nullptr;

    struct Sensor
    {
        int channel = 0; // Default to valid channel
        int address = InvalidAddress;
        bool invertCount = false;
    };

    Sensor mSensors[2] = {};

    int32_t mAxleCount = 0;
    State mState = State::OccupiedAtStart;
    bool mHasPower = false;

    QBasicTimer mResetTimer;
};

#endif // TRAINTASTIC_AXLE_COUNTER_OBJECT_H
