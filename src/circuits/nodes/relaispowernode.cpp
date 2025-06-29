/**
 * src/circuits/nodes/relaispowernode.cpp
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

#include "relaispowernode.h"

#include "../../objects/relais/model/abstractrelais.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"

#include <QTimerEvent>

#include <QJsonObject>

RelaisPowerNode::RelaisPowerNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 2 side (1 is optional)
    mContacts.append(NodeContact("41", "42"));
    mContacts.append(NodeContact("43", "44"));
}

RelaisPowerNode::~RelaisPowerNode()
{
    setRelais(nullptr);
}

AbstractCircuitNode::ConnectionsRes RelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact > 1)
        return {};

    if(!mHasSecondConnector && source.nodeContact != 0)
        return {};

    if(!relais())
        return{};

    if(relais()->relaisType() == AbstractRelais::RelaisType::Polarized)
    {
        // Polarized must have positive on first pole
        if((source.cable.pole != CircuitPole::First) != invertDir)
            return {};
    }
    else if(relais()->relaisType() == AbstractRelais::RelaisType::PolarizedInverted)
    {
        // Polarized inverted must have positive on second pole
        if((source.cable.pole != CircuitPole::Second) != invertDir)
            return {};
    }
    else if(relais()->relaisType() == AbstractRelais::RelaisType::Combinator)
    {
        // Bifilar circuit, only allow first pole in/out
        if(source.cable.pole != CircuitPole::First)
            return {};

        // Close the circuit between 2 connectors
        const int otherContact = source.nodeContact == 1 ? 0 : 1;
        CableItem comb;
        comb.cable.cable = mContacts.at(otherContact).cable;
        comb.cable.side = mContacts.at(otherContact).cableSide;
        comb.nodeContact = otherContact;
        comb.cable.pole = CircuitPole::First;
        return {comb};
    }

    // Close the circuit
    CableItem dest;
    dest.cable.cable = mContacts.at(source.nodeContact).cable;
    dest.cable.side = mContacts.at(source.nodeContact).cableSide;
    dest.nodeContact = source.nodeContact;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void RelaisPowerNode::addCircuit(ElectricCircuit *circuit)
{
    const bool wasActiveFirst = hasCircuit(0, CircuitType::Closed);
    const bool wasActiveSecond = mHasSecondConnector &&
            hasCircuit(1, CircuitType::Closed);

    AbstractCircuitNode::addCircuit(circuit);

    const bool isActiveFirst = hasCircuit(0, CircuitType::Closed);
    const bool isActiveSecond = mHasSecondConnector &&
            hasCircuit(1, CircuitType::Closed);

    const bool combinator = (relais() && relais()->relaisType() == AbstractRelais::RelaisType::Combinator);

    if(isActiveFirst && !wasActiveFirst)
    {
        if(combinator)
        {
            activateRelay(mCombinatorSecondCoil ? 0 : 1);
        }
        else
        {
            activateRelay(0);
        }
    }

    if(!combinator && mHasSecondConnector && isActiveSecond && !wasActiveSecond)
    {
        activateRelay(1);
    }
}

void RelaisPowerNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const bool wasActiveFirst = hasCircuit(0, CircuitType::Closed);
    const bool wasActiveSecond = mHasSecondConnector &&
            hasCircuit(1, CircuitType::Closed);

    AbstractCircuitNode::removeCircuit(circuit, items);

    const bool isActiveFirst = hasCircuit(0, CircuitType::Closed);
    const bool isActiveSecond = mHasSecondConnector &&
            hasCircuit(1, CircuitType::Closed);

    const bool combinator = (relais() && relais()->relaisType() == AbstractRelais::RelaisType::Combinator);

    if(!isActiveFirst && wasActiveFirst)
    {
        if(combinator)
        {
            deactivateRelay(mCombinatorSecondCoil ? 0 : 1);
        }
        else
        {
            deactivateRelay(0);
        }
    }

    if(!combinator && mHasSecondConnector && !isActiveSecond && wasActiveSecond)
    {
        deactivateRelay(1);
    }
}

bool RelaisPowerNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    auto model = modeMgr()->modelForType(AbstractRelais::Type);
    if(model)
    {
        const QString relaisName = obj.value("relais").toString();
        AbstractSimulationObject *relayObj = model->getObjectByName(relaisName);
        setRelais(static_cast<AbstractRelais *>(relayObj));
    }
    else
        setRelais(nullptr);

    mDelayUpSeconds = obj.value("delay_up_sec").toInt();
    mDelayDownSeconds = obj.value("delay_down_sec").toInt();

    setHasSecondConnector(obj.value("has_second_connector").toBool());

    setCombinatorSecondCoil(obj.value("combinator_second_coil").toBool());

    return true;
}

void RelaisPowerNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();

    obj["delay_up_sec"] = mDelayUpSeconds;
    obj["delay_down_sec"] = mDelayDownSeconds;

    obj["has_second_connector"] = hasSecondConnector();
    obj["combinator_second_coil"] = combinatorSecondCoil();
}

void RelaisPowerNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty relProp;
    relProp.name = "relais";
    relProp.prettyName = tr("Relay");
    relProp.types = {AbstractRelais::Type};
    result.append(relProp);
}

QString RelaisPowerNode::nodeType() const
{
    return NodeType;
}

bool RelaisPowerNode::tryFlipNode(bool forward)
{
    if(!mRelais || mRelais->relaisType() != AbstractRelais::RelaisType::Combinator)
        return false;

    // If we are Combinator relay, flip coil
    setCombinatorSecondCoil(!combinatorSecondCoil());
    return true;
}

AbstractRelais *RelaisPowerNode::relais() const
{
    return mRelais;
}

void RelaisPowerNode::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;

    stopTimer(0);
    stopTimer(1);

    if(mRelais)
    {
        if(mIsUp[0])
            mRelais->powerNodeDeactivated(this, false);
        if(mHasSecondConnector && mIsUp[1])
            mRelais->powerNodeDeactivated(this, true);

        mRelais->removePowerNode(this);

        disconnect(mRelais, &AbstractRelais::typeChanged,
                   this, &RelaisPowerNode::onRelayTypeChanged);
    }

    mIsUp[0] = false;
    mIsUp[1] = false;

    mRelais = newRelais;

    // Check new relay type
    onRelayTypeChanged();

    if(mRelais)
    {
        mRelais->addPowerNode(this);

        if(hasCircuit(0, CircuitType::Closed))
            activateRelay(0);
        if(mHasSecondConnector && hasCircuit(1, CircuitType::Closed))
            activateRelay(1);

        connect(mRelais, &AbstractRelais::typeChanged,
                this, &RelaisPowerNode::onRelayTypeChanged);
    }

    emit relayChanged(mRelais);
    modeMgr()->setFileEdited();
}

int RelaisPowerNode::delayUpSeconds() const
{
    return mDelayUpSeconds;
}

void RelaisPowerNode::setDelayUpSeconds(int newDelayUpSeconds)
{
    if(mDelayUpSeconds == newDelayUpSeconds)
        return;

    mDelayUpSeconds = newDelayUpSeconds;
    emit delaysChanged();
    emit shapeChanged();
    modeMgr()->setFileEdited();
}

int RelaisPowerNode::delayDownSeconds() const
{
    return mDelayDownSeconds;
}

void RelaisPowerNode::setDelayDownSeconds(int newDelayDownSeconds)
{
    if(mDelayDownSeconds == newDelayDownSeconds)
        return;

    mDelayDownSeconds = newDelayDownSeconds;
    emit delaysChanged();
    emit shapeChanged();
    modeMgr()->setFileEdited();
}

void RelaisPowerNode::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mTimerIds[0] || e->timerId() == mTimerIds[1])
    {
        Q_ASSERT_X(mRelais, "RelaisPowerNode::timerEvent", "no relay");
        const int contact = (e->timerId() == mTimerIds[0]) ? 0 : 1;

        // Do delayed action
        mIsUp[contact] = wasGoingUp[contact];
        if(wasGoingUp[contact])
            mRelais->powerNodeActivated(this, contact == 1);
        else
            mRelais->powerNodeDeactivated(this, contact == 1);

        stopTimer(contact);
        return;
    }
    else if(e->timerId() == mPercentTimerId)
    {
        Q_ASSERT_X(mRelais, "RelaisPowerNode::timerEvent", "no relay");

        // Increment timeout percent (down is negative)
        const double upIncrement = 1.0 / (4.0 * qMax(0.1, double(mDelayUpSeconds)));
        const double downIncrement = -1.0 / (4.0 * qMax(0.1, double(mDelayDownSeconds)));

        if(mTimerIds[0])
            mTimeoutPercentStatus[0] += wasGoingUp[0] ? upIncrement : downIncrement;
        if(mTimerIds[1])
            mTimeoutPercentStatus[1] += wasGoingUp[1] ? upIncrement : downIncrement;

        // Update visual drawing
        emit circuitsChanged();
        return;
    }

    AbstractCircuitNode::timerEvent(e);
}

void RelaisPowerNode::activateRelay(int contact)
{
    Q_ASSERT(contact == 0 || contact == 1);

    if(mTimerIds[contact] && wasGoingUp[contact])
        return; // Already scheduled

    stopTimer(contact);

    if(mIsUp[contact] || !mRelais)
        return; // Already in position

    if(mDelayUpSeconds == 0)
    {
        // Do it now
        mIsUp[contact] = true;
        mRelais->powerNodeActivated(this, contact == 1);
    }
    else
    {
        wasGoingUp[contact] = true;
        mTimerIds[contact] = startTimer(mDelayUpSeconds * 1000,
                                        Qt::PreciseTimer);
        mTimeoutPercentStatus[contact] = 0.0; // We start from bottom
        ensureTimeoutPercentTimer();
    }
}

void RelaisPowerNode::deactivateRelay(int contact)
{
    Q_ASSERT(contact == 0 || contact == 1);

    if(mTimerIds[contact] && !wasGoingUp[contact])
        return; // Already scheduled

    stopTimer(contact);

    if(!mIsUp[contact] || !mRelais)
        return; // Already in position

    if(mDelayDownSeconds == 0)
    {
        // Do it now
        mIsUp[contact] = false;
        mRelais->powerNodeDeactivated(this, contact == 1);
    }
    else
    {
        wasGoingUp[contact] = false;
        mTimerIds[contact] = startTimer(mDelayDownSeconds * 1000,
                                        Qt::PreciseTimer);
        mTimeoutPercentStatus[contact] = 1.0; // We start from top
        ensureTimeoutPercentTimer();
    }
}

void RelaisPowerNode::stopTimer(int contact)
{
    Q_ASSERT(contact == 0 || contact == 1);

    if(!mTimerIds[contact])
        return;

    killTimer(mTimerIds[contact]);
    mTimerIds[contact] = 0;

    if(!mTimerIds[0] && !mTimerIds[1])
    {
        stopTimeoutPercentTimer();
    }
}

void RelaisPowerNode::ensureTimeoutPercentTimer()
{
    if(mPercentTimerId)
        return;

    // Start an auxiliary timer which increases
    // percent of elapsed time since delay started
    mPercentTimerId = startTimer(250, Qt::CoarseTimer);
}

void RelaisPowerNode::stopTimeoutPercentTimer()
{
    if(!mPercentTimerId)
        return;

    killTimer(mPercentTimerId);
    mPercentTimerId = 0;

    // Reset percent status
    mTimeoutPercentStatus[0] = 0.0;
    mTimeoutPercentStatus[1] = 0.0;

    // Update visual drawing
    emit circuitsChanged();
}

bool RelaisPowerNode::combinatorSecondCoil() const
{
    return mCombinatorSecondCoil;
}

void RelaisPowerNode::setCombinatorSecondCoil(bool newCombinatorSecondCoil)
{
    if(mCombinatorSecondCoil == newCombinatorSecondCoil)
        return;

    mCombinatorSecondCoil = newCombinatorSecondCoil;

    emit shapeChanged();
    modeMgr()->setFileEdited();
}

bool RelaisPowerNode::hasSecondConnector() const
{
    return mHasSecondConnector;
}

void RelaisPowerNode::setHasSecondConnector(bool newHasSecondConnector)
{
    if(mRelais)
    {
        if(mRelais->mustHaveTwoConnectors())
            newHasSecondConnector = true;

        if(!mRelais->canHaveTwoConnectors())
            newHasSecondConnector = false;
    }

    if(mHasSecondConnector == newHasSecondConnector)
        return;

    mHasSecondConnector = newHasSecondConnector;

    if(!mHasSecondConnector)
    {
        // Circuits must be disabled before editing contacts
        Q_ASSERT(getCircuits(CircuitType::Closed).isEmpty());
        Q_ASSERT(getCircuits(CircuitType::Open).isEmpty());

        // Detach cable from second connector
        detachCable(1);
    }

    emit shapeChanged();
    modeMgr()->setFileEdited();
}

void RelaisPowerNode::onRelayTypeChanged()
{
    if(!mRelais)
        return;

    // Update having second connector
    setHasSecondConnector(hasSecondConnector());

    // Emit regardless to update visual drawing
    emit shapeChanged();
}
