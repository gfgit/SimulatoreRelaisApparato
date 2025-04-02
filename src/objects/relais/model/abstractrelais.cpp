/**
 * src/objects/relais/model/abstractrelais.cpp
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

#include "abstractrelais.h"

#include "../../../circuits/nodes/relaiscontactnode.h"
#include "../../../circuits/nodes/relaispowernode.h"

#include <QTimerEvent>

#include <QJsonObject>

#include <QRandomGenerator>

#include <QCoreApplication> // for postEvent()

class DelayedSetStateEvent : public QEvent
{
public:
    static const QEvent::Type _Type = QEvent::Type(QEvent::User + 1);

    DelayedSetStateEvent(AbstractRelais::State s) : QEvent(_Type), mState(s) {};

    inline AbstractRelais::State getState() const { return mState; }

private:
    AbstractRelais::State mState = AbstractRelais::State::Down;
};

QString AbstractRelais::getRelaisTypeName(RelaisType t)
{
    switch(t)
    {
    case RelaisType::Normal:
        return tr("Normal");
    case RelaisType::Polarized:
        return tr("Polarized");
    case RelaisType::PolarizedInverted:
        return tr("Polarized Inverted");
    case RelaisType::Stabilized:
        return tr("Stabilized");
    case RelaisType::Combinator:
        return tr("Combinator");
    case RelaisType::Timer:
        return tr("Timer");
    case RelaisType::Blinker:
        return tr("Blinker");
    case RelaisType::Encoder:
        return tr("Encoder");
    case RelaisType::Decoder:
        return tr("Decoder");
    case RelaisType::CodeRepeater:
        return tr("Code Repeater");
    case RelaisType::NTypes:
        break;
    }

    return QString();
}

AbstractRelais::AbstractRelais(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{

}

AbstractRelais::~AbstractRelais()
{
    auto powerNodes = mPowerNodes;
    for(RelaisPowerNode *p : powerNodes)
    {
        p->setRelais(nullptr);
    }

    auto contactNodes = mContactNodes;
    for(RelaisContactNode *c : contactNodes)
    {
        c->setRelais(nullptr);
    }

    killTimer(mTimerId);
    mTimerId = 0;
}

bool AbstractRelais::event(QEvent *e)
{
    if(e->type() == DelayedSetStateEvent::_Type)
    {
        setState(static_cast<DelayedSetStateEvent *>(e)->getState());
        return true;
    }

    return AbstractSimulationObject::event(e);
}

QString AbstractRelais::getType() const
{
    return Type;
}

bool AbstractRelais::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    setRelaisType(RelaisType(obj.value("relay_type").toInt(int(RelaisType::Normal))));
    setDurationUp(quint32(obj.value("duration_up_ms").toInteger()));
    setDurationDown(quint32(obj.value("duration_down_ms").toInteger()));
    setNormallyUp(obj.value("normally_up").toBool());

    return true;
}

void AbstractRelais::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    if(mCustomUpMS > 0)
        obj["duration_up_ms"] = qint64(mCustomUpMS);
    if(mCustomDownMS > 0)
        obj["duration_down_ms"] = qint64(mCustomDownMS);
    obj["normally_up"] = normallyUp();
    obj["relay_type"] = int(relaisType());
}

int AbstractRelais::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int nodesCount = AbstractSimulationObject::getReferencingNodes(result);

    nodesCount += mContactNodes.size() + mPowerNodes.size();

    if(result)
    {
        for(auto item : mContactNodes)
            result->append(item);
        for(auto item : mPowerNodes)
            result->append(item);
    }

    return nodesCount;
}

bool AbstractRelais::isStateIndependent(RelaisType t)
{
    switch(t)
    {
    case AbstractRelais::RelaisType::Stabilized:
    case AbstractRelais::RelaisType::Combinator:
        return true;
    default:
        break;
    }

    return false;
}

quint32 AbstractRelais::durationUp() const
{
    return mCustomUpMS;
}

void AbstractRelais::setDurationUp(quint32 durationUpMS)
{
    if(mCustomUpMS == durationUpMS)
        return;

    mCustomUpMS = durationUpMS;
    emit settingsChanged(this);
}

quint32 AbstractRelais::durationDown() const
{
    return mCustomDownMS;
}

void AbstractRelais::setDurationDown(quint32 durationDownMS)
{
    if(mCustomDownMS == durationDownMS)
        return;

    mCustomDownMS = durationDownMS;
    emit settingsChanged(this);
}

void AbstractRelais::addPowerNode(RelaisPowerNode *p)
{
    Q_ASSERT_X(!mPowerNodes.contains(p),
               "addPowerNode", "already added");

    mPowerNodes.append(p);

    emit nodesChanged(this);
}

void AbstractRelais::removePowerNode(RelaisPowerNode *p)
{
    Q_ASSERT_X(mPowerNodes.contains(p),
               "removePowerNode", "not registered");
    Q_ASSERT_X(p->relais() == this,
               "removePowerNode", "relay does not match");

    mPowerNodes.removeOne(p);

    emit nodesChanged(this);
}

void AbstractRelais::addContactNode(RelaisContactNode *c)
{
    Q_ASSERT_X(!mContactNodes.contains(c),
               "addContactNode", "already added");

    mContactNodes.append(c);

    emit nodesChanged(this);
}

void AbstractRelais::removeContactNode(RelaisContactNode *c)
{
    Q_ASSERT_X(mContactNodes.contains(c),
               "removeContactNode", "not registered");
    Q_ASSERT_X(c->relais() == this,
               "removeContactNode", "relay does not match");

    mContactNodes.removeOne(c);

    emit nodesChanged(this);
}

void AbstractRelais::timerEvent(QTimerEvent *e)
{
    if(mTimerId && e->timerId() == mTimerId)
    {
        if(relaisType() == RelaisType::Decoder)
        {
            setDecodedResult(0, false);
            return;
        }

        if(relaisType() == RelaisType::Blinker || relaisType() == RelaisType::Encoder)
        {
            if(mActivePowerNodesUp == 0)
            {
                // Go down and stop timer
                killTimer(mTimerId);
                mTimerId = 0;

                setState(State::Down);
                return;
            }

            // Invert state
            switch (state())
            {
            case State::Down:
            case State::GoingDown:
            case State::GoingUp:
                setState(State::Up);
                break;
            case State::Up:
                setState(State::Down);
            default:
                break;
            }

            if(relaisType() == RelaisType::Blinker && mCustomDownMS > 0)
            {
                // Asymmetric blink

                killTimer(mTimerId);
                mTimerId = startTimer(state() == State::Down ? mCustomDownMS : mCustomUpMS,
                                      Qt::PreciseTimer);
            }

            return;
        }

        double newPosition = mPosition;
        if(mInternalState == State::GoingUp)
            newPosition += mTickPositionDelta;
        else if(mInternalState == State::GoingDown)
            newPosition -= mTickPositionDelta;
        else
        {
            killTimer(mTimerId);
            mTimerId = 0;
        }

        if((newPosition < 0.0) || (newPosition > 1.0))
        {
            mInternalState = mState;
            killTimer(mTimerId);
            mTimerId = 0;
        }

        setPosition(newPosition);

        return;
    }

    QObject::timerEvent(e);
}

void AbstractRelais::powerNodeActivated(RelaisPowerNode *p, bool secondContact)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->relais() == this);

    const bool hadActiveUp = mActivePowerNodesUp > 0;
    const bool hadActiveDown = mActivePowerNodesDown > 0;

    if(stateIndependent() && secondContact)
    {
        mActivePowerNodesDown++;
    }
    else
    {
        mActivePowerNodesUp++;
    }

    if(stateIndependent())
    {
        if(mActivePowerNodesDown == 0)
        {
            // Relay can go up only if down coil is not active
            if((hadActiveDown || !hadActiveUp) && mActivePowerNodesUp > 0)
            {
                // Up coil was activated
                // Or both coil were active but now down coil is deactivated

                // So relay anchor goes up
                startMove(true);
            }
        }
        else if(!hadActiveDown)
        {
            // Down circuit wins always if present
            startMove(false);
        }
    }
    else if(mActivePowerNodesUp == 1)
    {
        if(relaisType() == RelaisType::Blinker || relaisType() == RelaisType::Encoder)
        {
            if(mTimerId)
                killTimer(mTimerId);

            // Timer will switch state
            if(relaisType() == RelaisType::Encoder)
            {
                if(mCustomUpMS > 0)
                {
                    mTimerId = startTimer(timeoutMillisForCode(getCode()),
                                          Qt::PreciseTimer);
                }
            }
            else
            {
                mTimerId = startTimer(mCustomDownMS > 0 ? mCustomDownMS : mCustomUpMS,
                                      Qt::PreciseTimer);
            }
        }
        else if(relaisType() == RelaisType::Decoder)
        {
            if(mElapsedTimer.isValid())
            {
                const qint64 elapsed = mElapsedTimer.restart();

                const quint32 code = codeForMillis(elapsed);

                // If no/wrong code detected, skip another cicle
                if(code == 0 || (getDecodedCode() != 0 && code != getDecodedCode()))
                {
                    mElapsedTimer.invalidate();
                    setDecodedResult(0, true);
                }
                else
                {
                    setDecodedResult(code, true);
                }

            }
            else
            {
                mElapsedTimer.start();
                setDecodedResult(0, true);
            }
        }
        else if(relaisType() == AbstractRelais::RelaisType::CodeRepeater)
        {
            // This relay is super fast
            QCoreApplication::postEvent(this, new DelayedSetStateEvent(State::Up));
        }
        else
        {
            // Begin powering normal relais
            startMove(true);
        }
    }

    // NOTE: we do not emit stateChanged(this) signal
    // because we are called from inside circuit creation logic.
    // This could allow recursion and trigger asserts.
    // Since we did startMove() we let timerEvent() emit it.
}

void AbstractRelais::powerNodeDeactivated(RelaisPowerNode *p, bool secondContact)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->relais() == this);

    const bool hadActiveUp = mActivePowerNodesUp > 0;
    const bool hadActiveDown = mActivePowerNodesDown > 0;

    if(stateIndependent() && secondContact)
    {
        Q_ASSERT_X(mActivePowerNodesDown > 0,
                   "powerNodeDeactivated",
                   "none active down");
        mActivePowerNodesDown--;
    }
    else
    {
        Q_ASSERT_X(mActivePowerNodesUp > 0,
                   "powerNodeDeactivated",
                   "none active up");
        mActivePowerNodesUp--;
    }

    if(stateIndependent())
    {
        // Relay can go up only if down coil is not active
        if(mActivePowerNodesDown == 0)
        {
            double UpThreshold = 0.5;
            if(relaisType() == RelaisType::Stabilized)
                UpThreshold = 0.7; // Stabilized magnet is not so powerful

            if((hadActiveDown || !hadActiveUp) && (mActivePowerNodesUp > 0 || mPosition > 0.5))
            {
                // If tried to go down but not yet reached middle position,
                // go back up.
                // Keep going up for permanent magnet/mechanical inertia
                // only if after middle position.
                if(mActivePowerNodesUp > 0 || mPosition > UpThreshold)
                    startMove(true);
            }
            else if(hadActiveUp && mPosition < UpThreshold)
            {
                // We tried to go up and then power was lost.
                startMove(false);
            }
        }
        else if(!hadActiveDown)
        {
            // Down circuit wins always if present
            startMove(false);
        }
    }
    else if(mActivePowerNodesUp == 0)
    {
        if(relaisType() == RelaisType::Blinker || relaisType() == RelaisType::Encoder)
        {
            // Timer will self stop at next timeout and set relais to Down state
        }
        else if(relaisType() == RelaisType::Decoder)
        {
            if(mElapsedTimer.isValid())
            {
                const qint64 elapsed = mElapsedTimer.restart();

                const quint32 code = codeForMillis(elapsed);
                setDecodedResult(code, true);
            }
            else
            {
                mElapsedTimer.start();
                setDecodedResult(0, true);
            }
        }
        else if(relaisType() == AbstractRelais::RelaisType::CodeRepeater)
        {
            // This relay is super fast
            QCoreApplication::postEvent(this, new DelayedSetStateEvent(State::Down));
        }
        else
        {
            // End powering normal relais
            startMove(false);
        }
    }

    // NOTE: see powerNodeActivated() comment
}

void AbstractRelais::setPosition(double newPosition)
{
    newPosition = qBound(0.0, newPosition, 1.0);

    if(qFuzzyCompare(mPosition, newPosition))
        return;

    const bool up = newPosition > mPosition;

    mPosition = newPosition;

    if(mPosition < 0.1)
        setState(State::Down);
    else if(mPosition > 0.9)
        setState(State::Up);
    else if(up)
        setState(State::GoingUp);
    else
        setState(State::GoingDown);
}

void AbstractRelais::startMove(bool up)
{
    if(mTimerId)
        killTimer(mTimerId);

    mInternalState = up ? State::GoingUp : State::GoingDown;

    int totalTime = 0;
    if(relaisType() == RelaisType::Combinator)
    {
        // Combinators have same time in either directions
        totalTime = mCustomUpMS;
        if(!totalTime)
            totalTime = DefaultCombinatorMS;
    }
    else if(up || relaisType() == RelaisType::Combinator)
    {
        totalTime = mCustomUpMS;
        if(!totalTime)
            totalTime = DefaultUpMS;
    }
    else
    {
        totalTime = mCustomDownMS;
        if(!totalTime)
            totalTime = DefaultDownMS;
    }

    // Relay can be up to 5% faster/slower
    const double MaxTimeOscillationRel = 0.05;
    const double factor =
            QRandomGenerator::global()->bounded(MaxTimeOscillationRel * 2) - MaxTimeOscillationRel;
    const int timeOscillation = qRound(double(totalTime) * factor);

    totalTime += timeOscillation;

    // Get a sensible tick duration
    const int tickDurationMS = qBound(20, totalTime / 10, 200);

    // How much to change position at each tick
    mTickPositionDelta = double(tickDurationMS) / double(totalTime);

    mTimerId = startTimer(tickDurationMS);
}

void AbstractRelais::setDecodedResult(quint32 code, bool delay)
{
    Q_ASSERT(relaisType() == RelaisType::Decoder);

    mCustomDownMS = code;

    killTimer(mTimerId);
    mTimerId = 0;

    if(code == 0 || (getCode() != 0 && code != getCode()))
    {
        // No code or wrong code detected
        if(delay)
            QCoreApplication::postEvent(this, new DelayedSetStateEvent(State::Down));
        else
            setState(State::Down);
        return;
    }

    // We detected correct code
    if(delay)
        QCoreApplication::postEvent(this, new DelayedSetStateEvent(State::Up));
    else
        setState(State::Up);

    // Start timer to end code detection
    mTimerId = startTimer(timeoutMillisForCode(code) + CodeErrorMarginMillis);
}

quint32 AbstractRelais::codeForMillis(qint64 millis)
{
    if(std::abs(millis - timeoutMillisForCode(75)) <= CodeErrorMarginMillis)
        return 75;
    if(std::abs(millis - timeoutMillisForCode(120)) <= CodeErrorMarginMillis)
        return 120;
    if(std::abs(millis - timeoutMillisForCode(180)) <= CodeErrorMarginMillis)
        return 180;
    if(std::abs(millis - timeoutMillisForCode(270)) <= CodeErrorMarginMillis)
        return 270;
    return 0;
}

AbstractRelais::RelaisType AbstractRelais::relaisType() const
{
    return mType;
}

void AbstractRelais::setRelaisType(RelaisType newType)
{
    if(mType == newType)
        return;

    const RelaisType oldType = mType;
    mType = newType;

    switch (oldType)
    {
    case RelaisType::Blinker:
    case RelaisType::Encoder:
    case RelaisType::Decoder:
    case RelaisType::CodeRepeater:
    {
        // Reset to default
        mCustomUpMS = 0;
        mCustomDownMS = 0;
        mElapsedTimer.invalidate();

        if(mTimerId)
        {
            killTimer(mTimerId);
            mTimerId = 0;

            setState(State::Down);
        }
        break;
    }
    default:
        break;
    }

    if(newType == RelaisType::Blinker)
    {
        // Default to 1000 up and symmetric down time
        mCustomUpMS = 1000;
        mCustomDownMS = 0;

        setState(State::Down);
    }
    else if(newType == RelaisType::Encoder || newType == RelaisType::Decoder)
    {
        mCustomUpMS = 75;
        mCustomDownMS = 0;

        setState(State::Down);
    }
    else if(newType == RelaisType::CodeRepeater)
    {
        mCustomUpMS = 0;
        mCustomDownMS = 0;
        setState(State::Down);
    }

    if(stateIndependent())
    {
        // For stabilized relais we start in normal condition
        if(mNormallyUp)
            setPosition(1.0);
        else
            setPosition(0.0);
    }
    else
    {
        setPosition(0.0); // Start from Down
    }

    emit settingsChanged(this);
    emit typeChanged(this, mType);
}

bool AbstractRelais::normallyUp() const
{
    return mNormallyUp;
}

void AbstractRelais::setNormallyUp(bool newNormallyUp)
{
    if(mNormallyUp == newNormallyUp)
        return;

    mNormallyUp = newNormallyUp;

    if(stateIndependent())
    {
        // For stabilized relais we start in normal condition
        if(mNormallyUp)
            setPosition(1.0);
        else
            setPosition(0.0);
    }

    emit settingsChanged(this);

    // Update nodes
    for(RelaisPowerNode *p : mPowerNodes)
    {
        emit p->shapeChanged();
    }

    for(RelaisContactNode *c : mContactNodes)
    {
        emit c->shapeChanged();
    }
}

AbstractRelais::State AbstractRelais::state() const
{
    return mState;
}

void AbstractRelais::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;
    emit stateChanged(this);
}

QString AbstractRelais::getStateName() const
{
    if(relaisType() == RelaisType::Combinator)
    {
        // Combinator Relay has different state names
        switch (state())
        {
        case State::Up:
            return tr("Reverse");
        case State::Down:
            return tr("Normal");
        case State::GoingUp:
            return tr("Going reverse");
        case State::GoingDown:
            return tr("Going normal");
        default:
            break;
        }
    }
    else
    {
        switch (state())
        {
        case State::Up:
            return tr("Up");
        case State::Down:
            return tr("Down");
        case State::GoingUp:
            return tr("Going up");
        case State::GoingDown:
            return tr("Going down");
        default:
            break;
        }
    }

    return QString();
}
