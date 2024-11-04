/**
 * src/nodes/relaispowernode.cpp
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

#include "../objects/relais/model/abstractrelais.h"
#include "../objects/relais/model/relaismodel.h"

#include "../views/modemanager.h"

#include <QTimerEvent>

#include <QJsonObject>

RelaisPowerNode::RelaisPowerNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 1 side
    mContacts.append(NodeContact("41", "42"));
}

RelaisPowerNode::~RelaisPowerNode()
{
    setRelais(nullptr);
}

QVector<CableItem> RelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0)
        return {};

    // Close the circuit
    // TODO: polarized relays?
    CableItem dest;
    dest.cable.cable = mContacts.at(0).cable;
    dest.cable.side = mContacts.at(0).cableSide;
    dest.nodeContact = 0;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void RelaisPowerNode::addCircuit(ElectricCircuit *circuit)
{
    const CircuitList &closedCircuits = getCircuits(CircuitType::Closed);
    bool wasEmpty = closedCircuits.isEmpty();

    AbstractCircuitNode::addCircuit(circuit);

    if(wasEmpty && !closedCircuits.isEmpty())
    {
        activateRelay();
    }
}

void RelaisPowerNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const CircuitList &closedCircuits = getCircuits(CircuitType::Closed);
    bool hadCircuit = !closedCircuits.isEmpty();

    AbstractCircuitNode::removeCircuit(circuit, items);

    if(hadCircuit && closedCircuits.isEmpty())
    {
        deactivateRelay();
    }
}

bool RelaisPowerNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString relaisName = obj.value("relais").toString();

    setRelais(modeMgr()->relaisModel()->getRelay(relaisName));

    mDelayUpSeconds = obj["delay_up_sec"].toInt();
    mDelayDownSeconds = obj["delay_down_sec"].toInt();

    return true;
}

void RelaisPowerNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();

    obj["delay_up_sec"] = mDelayUpSeconds;
    obj["delay_down_sec"] = mDelayDownSeconds;
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

    stopTimer();

    if(mIsUp && mRelais)
    {
        mRelais->powerNodeDeactivated(this);
        mIsUp = false;
    }

    if(mRelais)
    {
        if(mIsUp)
            mRelais->powerNodeDeactivated(this);
        mRelais->removePowerNode(this);
    }

    mIsUp = false;
    mRelais = newRelais;

    if(mRelais)
    {
        mRelais->addPowerNode(this);

        if(hasCircuits(CircuitType::Closed))
            activateRelay();
    }

    emit relayChanged(mRelais);
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
}

void RelaisPowerNode::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mTimerId)
    {
        Q_ASSERT(mRelais);

        // Do delayed action
        mIsUp = wasGoingUp;
        if(wasGoingUp)
            mRelais->powerNodeActivated(this);
        else
            mRelais->powerNodeDeactivated(this);

        stopTimer();
        return;
    }

    AbstractCircuitNode::timerEvent(e);
}

void RelaisPowerNode::activateRelay()
{
    if(mTimerId && wasGoingUp)
        return; // Already scheduled

    stopTimer();

    if(mIsUp || !mRelais)
        return; // Already in position

    if(mDelayUpSeconds == 0)
    {
        // Do it now
        mIsUp = true;
        mRelais->powerNodeActivated(this);
    }
    else
    {
        wasGoingUp = true;
        mTimerId = startTimer(mDelayUpSeconds * 1000,
                              Qt::PreciseTimer);
    }
}

void RelaisPowerNode::deactivateRelay()
{
    if(mTimerId && !wasGoingUp)
        return; // Already scheduled

    stopTimer();

    if(!mIsUp || !mRelais)
        return; // Already in position

    if(mDelayDownSeconds == 0)
    {
        // Do it now
        mIsUp = false;
        mRelais->powerNodeDeactivated(this);
    }
    else
    {
        wasGoingUp = false;
        mTimerId = startTimer(mDelayDownSeconds * 1000,
                              Qt::PreciseTimer);
    }
}

void RelaisPowerNode::stopTimer()
{
    if(!mTimerId)
        return;

    killTimer(mTimerId);
    mTimerId = 0;
}
