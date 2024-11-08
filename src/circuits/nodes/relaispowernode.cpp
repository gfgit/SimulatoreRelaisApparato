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
#include "../../objects/relais/model/relaismodel.h"

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

QVector<CableItem> RelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact > 1)
        return {};

    if(!mHasSecondConnector && source.nodeContact != 0)
        return {};

    if(!relais())
        return{};

    if(relais()->type() == AbstractRelais::Type::Polarized)
    {
        // Polarized must have positive on first pole
        if((source.cable.pole != CircuitPole::First) != invertDir)
            return {};
    }
    else if(relais()->type() == AbstractRelais::Type::PolarizedInverted)
    {
        // Polarized inverted must have positive on second pole
        if((source.cable.pole != CircuitPole::Second) != invertDir)
            return {};
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

    if(isActiveFirst && !wasActiveFirst)
    {
        activateRelay(0);
    }

    if(mHasSecondConnector && isActiveSecond && !wasActiveSecond)
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

    if(!isActiveFirst && wasActiveFirst)
    {
        deactivateRelay(0);
    }

    if(mHasSecondConnector && !isActiveSecond && wasActiveSecond)
    {
        deactivateRelay(1);
    }
}

bool RelaisPowerNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString relaisName = obj.value("relais").toString();

    setRelais(modeMgr()->relaisModel()->getRelay(relaisName));

    mDelayUpSeconds = obj.value("delay_up_sec").toInt();
    mDelayDownSeconds = obj.value("delay_down_sec").toInt();

    setHasSecondConnector(obj.value("has_second_connector").toBool());

    return true;
}

void RelaisPowerNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();

    obj["delay_up_sec"] = mDelayUpSeconds;
    obj["delay_down_sec"] = mDelayDownSeconds;

    obj["has_second_connector"] = hasSecondConnector();
}

QString RelaisPowerNode::nodeType() const
{
    return NodeType;
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
    modeMgr()->setFileEdited();
}

void RelaisPowerNode::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mTimerIds[0] || e->timerId() == mTimerIds[1])
    {
        Q_ASSERT(mRelais);
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
    }
}

void RelaisPowerNode::stopTimer(int contact)
{
    Q_ASSERT(contact == 0 || contact == 1);

    if(!mTimerIds[contact])
        return;

    killTimer(mTimerIds[contact]);
    mTimerIds[contact] = 0;
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

    emit shapeChanged();
    modeMgr()->setFileEdited();

    if(!mHasSecondConnector)
    {
        // Circuits must be disabled before editing contacts
        Q_ASSERT(getCircuits(CircuitType::Closed).isEmpty());
        Q_ASSERT(getCircuits(CircuitType::Open).isEmpty());

        // Detach cable from second connector
        detachCable(1);
    }
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
