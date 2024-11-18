/**
 * src/objects/relais/model/abstractrelais.h
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

#ifndef ABSTRACTRELAIS_H
#define ABSTRACTRELAIS_H

#include "../../abstractsimulationobject.h"

class RelaisPowerNode;
class RelaisContactNode;

class QJsonObject;

class AbstractRelais : public AbstractSimulationObject
{
    Q_OBJECT
public:

    enum class State
    {
        Up = 0,
        Down = 1,
        GoingUp = 2,
        GoingDown = 3
    };

    enum class RelaisType
    {
        Normal = 0,
        Polarized,
        PolarizedInverted,
        Stabilized,
        Combinator,
        NTypes
    };

    static QString getRelaisTypeName(RelaisType t);

    explicit AbstractRelais(AbstractSimulationObjectModel *m);
    ~AbstractRelais();

    static constexpr QLatin1String Type = QLatin1String("abstract_relais");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject& obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject& obj) const override;

    QVector<AbstractCircuitNode*> nodes() const override;

    static bool isStateIndependent(RelaisType t);

    inline bool stateIndependent() const
    {
        return isStateIndependent(relaisType());
    }

    inline bool canHaveTwoConnectors() const
    {
        if(relaisType() == RelaisType::Combinator)
            return false; // We use 2 different tiles for it
        return true;
    }

    inline bool mustHaveTwoConnectors() const
    {
        if(relaisType() == RelaisType::Stabilized)
            return true;
        return false;
    }

    State state() const;
    void setState(State newState);

    double upSpeed() const;
    void setUpSpeed(double newUpSpeed);

    double downSpeed() const;
    void setDownSpeed(double newDownSpeed);

    void timerEvent(QTimerEvent *e) override;

    bool normallyUp() const;
    void setNormallyUp(bool newNormallyUp);

    RelaisType relaisType() const;
    void setRelaisType(RelaisType newType);

    inline int hasActivePowerUp() const
    {
        return mActivePowerNodesUp > 0;
    }

    inline int hasActivePowerDown() const
    {
        return mActivePowerNodesDown > 0;
    }

signals:
    void typeChanged(AbstractRelais *self, RelaisType s);

private:
    friend class RelaisPowerNode;
    void addPowerNode(RelaisPowerNode *p);
    void removePowerNode(RelaisPowerNode *p);

    friend class RelaisContactNode;
    void addContactNode(RelaisContactNode *c);
    void removeContactNode(RelaisContactNode *c);

    void powerNodeActivated(RelaisPowerNode *p, bool secondContact);
    void powerNodeDeactivated(RelaisPowerNode *p, bool secondContact);

    void setPosition(double newPosition);
    void startMove(bool up);

private:
    RelaisType mType = RelaisType::Normal;

    State mState = State::Down;
    State mInternalState = State::Down;
    bool mNormallyUp = true;

    // Steps per 250ms (Speed of 0.25 means it goes up in 1 sec)
    double mUpSpeed = 0.18;
    double mDownSpeed = 0.25;
    double mPosition = 0.0;
    int mTimerId = 0;

    QVector<RelaisPowerNode *> mPowerNodes;
    int mActivePowerNodesUp = 0;
    int mActivePowerNodesDown = 0;

    QVector<RelaisContactNode *> mContactNodes;
};

#endif // ABSTRACTRELAIS_H
