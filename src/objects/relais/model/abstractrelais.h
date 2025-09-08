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
#include "../../../enums/signalaspectcodes.h"

#include <QElapsedTimer>
#include <QBasicTimer>

class RelaisPowerNode;
class RelaisContactNode;

class QJsonObject;

class AbstractRelais : public AbstractSimulationObject
{
    Q_OBJECT
public:
    static constexpr int DefaultUpMS = 700;
    static constexpr int DefaultDownMS = 200;

    // Combinators are very fast and symmetric time
    static constexpr int DefaultCombinatorMS = 200;

    // Decoders are very fast too and symmetric time
    static constexpr int DefaultDecoderMS = 50;

    // Disk relays are pretty fast, symmetric
    static constexpr int DefaultDiskMS = 150;

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
        Timer,
        Blinker,
        Encoder,
        Decoder,
        CodeRepeater,
        DiskRelayAC,
        NTypes
    };

    static QString getRelaisTypeName(RelaisType t);

    explicit AbstractRelais(AbstractSimulationObjectModel *m);
    ~AbstractRelais();

    static constexpr QLatin1String Type = QLatin1String("abstract_relais");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject& obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject& obj) const override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    bool setReplicaState(const QCborMap& replicaState) override;
    void getReplicaState(QCborMap& replicaState) const override;

    static bool isStateIndependent(RelaisType t);

    inline bool stateIndependent() const
    {
        return isStateIndependent(relaisType());
    }

    inline bool canHaveTwoConnectors() const
    {
        return true;
    }

    inline bool mustHaveTwoConnectors() const
    {
        if(relaisType() == RelaisType::Stabilized)
            return true;
        if(relaisType() == RelaisType::Combinator)
            return true;
        if(relaisType() == RelaisType::DiskRelayAC)
            return true;
        return false;
    }

    State state() const;
    void setState(State newState);

    QString getStateName() const;

    quint32 durationUp() const;
    void setDurationUp(quint32 durationUpMS);

    quint32 durationDown() const;
    void setDurationDown(quint32 durationDownMS);

    bool event(QEvent *e) override;
    void timerEvent(QTimerEvent *e) override;

    bool normallyUp() const;
    void setNormallyUp(bool newNormallyUp);

    RelaisType relaisType() const;
    void setRelaisType(RelaisType newType);

    SignalAspectCode getExpectedCode() const;
    void setExpectedCode(SignalAspectCode code);

    inline SignalAspectCode getDetectedCode() const
    {
        return mDetectedCode;
    }

    inline int hasActivePowerUp() const
    {
        return mActivePowerNodesUp > 0;
    }

    inline int hasActivePowerDown() const
    {
        return mActivePowerNodesDown > 0;
    }

    inline int getPowerNodesCount() const
    {
        return mPowerNodes.size();
    }

    inline int getContactNodesCount() const
    {
        return mContactNodes.size();
    }

    bool isDelayed(State dir) const;

signals:
    void typeChanged(AbstractRelais *self, RelaisType s);

protected:
    void onReplicaModeChanged(bool on) override;

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

    void setDecodedResult(SignalAspectCode code);

    void startCodeTimeout(SignalAspectCode code);

    void applyDecodedResult();

    static constexpr inline int timeoutMillisForCode(SignalAspectCode code)
    {
        int pulsePerMinute = codeToNumber(code);
        if (pulsePerMinute == 0)
            return 0;

        // Code is number of interruptions per minute
        // But interruption is a full cyle
        // We want state change 2 times per cycle so return half time
        return (30 * 1000) / pulsePerMinute;
    }

    static SignalAspectCode codeForMillis(qint64 millis);

    void redrawContactNodes();

private:
    RelaisType mType = RelaisType::Normal;

    State mState = State::Down;
    State mInternalState = State::Down;
    bool mNormallyUp = false;

    quint32 mCustomUpMS = 0;
    quint32 mCustomDownMS = 0;
    double mTickPositionDelta = 0;
    double mPosition = 0.0;
    QBasicTimer mPositionTimer;

    QVector<RelaisPowerNode *> mPowerNodes;
    int mActivePowerNodesUp = 0;
    int mActivePowerNodesDown = 0;

    QVector<RelaisContactNode *> mContactNodes;

    static constexpr int CodeErrorMarginMillis = 18;

    static constexpr double UpPositionThreshold = 0.75;
    static constexpr double DownPositionThreshold = 0.25;

    SignalAspectCode mExpectedCode = SignalAspectCode::CodeAbsent;
    SignalAspectCode mDetectedCode = SignalAspectCode::CodeAbsent;
    SignalAspectCode mNextDetectedCode = SignalAspectCode::CodeAbsent;
};

#endif // ABSTRACTRELAIS_H
