/**
 * src/objects/traintastic/traintasticturnoutobj.h
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

#ifndef TRAINTASTICTURNOUTOBJ_H
#define TRAINTASTICTURNOUTOBJ_H

#include "../abstractsimulationobject.h"

#include <QBasicTimer>

class TraintasticTurnoutNode;

class TraintasticTurnoutObj : public AbstractSimulationObject
{
    Q_OBJECT
public:
    static constexpr int InvalidChannel = -1;
    static constexpr int InvalidAddress = -1;
    static constexpr int TickMillis = 200;

    enum State
    {
        Unknown = 0, // Middle
        Closed = 1,  // Straight
        Thrown = 2   // Curve
    };

    static constexpr double ClosetThreshold = 0.2;
    static constexpr double ThrownThreshold = 0.8;

    explicit TraintasticTurnoutObj(AbstractSimulationObjectModel *m);
    ~TraintasticTurnoutObj();

    static constexpr QLatin1String Type = QLatin1String("traintastic_turnout");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    inline int channel() const { return mChannel; }
    inline int address() const { return mAddress; }
    inline State state() const { return mState; }

    void setChannel(int newChannel);
    void setAddress(int newAddress);

    inline State initialState() const { return mInitialState; }

    void setInitialState(State newInitialState);

    int totalTimeMillis() const;
    void setTotalTimeMillis(int newTotalTimeMillis);

    inline bool isActive() const { return mTimer.isActive(); }

protected:
    void timerEvent(QTimerEvent *ev) override;

private:
    friend class TraintasticTurnoutNode;
    void setNode(TraintasticTurnoutNode *node);
    void setActive(bool val, bool up);

    void setPosition(double newPos);
    void setState(State newState);

private:
    TraintasticTurnoutNode *mNode = nullptr;

    int mChannel = 0;
    int mAddress = InvalidAddress;
    State mState = State::Closed;
    State mInitialState = State::Closed;
    int mTotalTimeMillis = 8000; // 8 seconds
    double mPosition = 0.0;
    bool isGoingUp = false;

    QBasicTimer mTimer;
};

#endif // TRAINTASTICTURNOUTOBJ_H
