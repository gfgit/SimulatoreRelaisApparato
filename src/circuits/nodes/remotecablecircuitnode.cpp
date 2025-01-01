/**
 * src/circuits/nodes/remotecablecircuitnode.cpp
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

#include "remotecablecircuitnode.h"

#include "../electriccircuit.h"

#include <QTimer>

static RemoteCableCircuitNode *otherRemote = nullptr;

RemoteCableCircuitNode::RemoteCableCircuitNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));

    // TODO: HACK
    if(otherRemote)
    {
        setLocalPeer(otherRemote);
        otherRemote = nullptr;
    }
    else
    {
        otherRemote = this;
    }
}

RemoteCableCircuitNode::~RemoteCableCircuitNode()
{
    setLocalPeer(nullptr);

    // TODO: HACK
    if(otherRemote == this)
        otherRemote = nullptr;
}

QVector<CableItem> RemoteCableCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0)
        return {};

    if(mMode != Mode::SendCurrentWaitClosed &&
            mMode != Mode::SendCurrentClosed)
        return {}; // Leave circuit open

    // Close the circuit
    CableItem dest;
    dest.cable.cable = mContacts.at(source.nodeContact).cable;
    dest.cable.side = mContacts.at(source.nodeContact).cableSide;
    dest.nodeContact = source.nodeContact;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void RemoteCableCircuitNode::addCircuit(ElectricCircuit *circuit)
{
    mStateDirty = false;

    const AnyCircuitType before = hasAnyCircuit(0);

    if(circuit->type() == CircuitType::Open &&
            circuit->getSource() == this && circuit->getEnd() == this
            && !getCircuits(CircuitType::Open).contains(circuit))
    {
        mFakeClosedCircuitsCount++;
    }

    AbstractCircuitNode::addCircuit(circuit);

    const AnyCircuitType after = hasAnyCircuit(0);

    if(before != after)
    {
        switch (mMode)
        {
        case Mode::None:
        {
            if(before == AnyCircuitType::None && after == AnyCircuitType::Open)
            {
                const auto items = circuit->getNode(this);
                mSendPole = items.first().fromPole;

                setMode(Mode::SendCurrentOpen);
            }
            break;
        }
        case Mode::SendCurrentWaitClosed:
        {
            if(before == AnyCircuitType::Open && after == AnyCircuitType::Closed)
            {
                setMode(Mode::SendCurrentClosed);
            }
            break;
        }
        case Mode::ReceiveCurrentWaitClosed:
        {
            if(before == AnyCircuitType::Open && after == AnyCircuitType::Closed)
            {
                setMode(Mode::ReceiveCurrentClosed);
            }
            break;
        }
        default:
            break;
        }
    }

    if(mFakeClosedCircuitsCount == 1 && mMode == Mode::ReceiveCurrentOpen)
    {
        // We got first fake closed circuit
        // This circuit can close, but we wait for send side to close too
        setMode(Mode::ReceiveCurrentWaitClosed);
    }
}

void RemoteCableCircuitNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const AnyCircuitType before = hasAnyCircuit(0);

    AbstractCircuitNode::removeCircuit(circuit, items);

    const AnyCircuitType after = hasAnyCircuit(0);

    if(before != after)
    {
        if(after == AnyCircuitType::None)
        {
            if(before == AnyCircuitType::Closed)
            {
                // When a closed circuit is interrupted AFTER passing this node
                // it is removed and then an open circuit replacement is added
                // so there is a time window where no circuits are registered on this node

                // So we cannot yet set Mode to None, otherwise when open circuit
                // gets registered it will be cut, as node will not be active.
                // Still tell peer we do not have closed circuit anymore

                if(mMode == Mode::SendCurrentClosed)
                    setMode(Mode::SendCurrentWaitClosed);
                else if(mMode == Mode::ReceiveCurrentClosed)
                    setMode(Mode::ReceiveCurrentOpen);

                scheduleStateRefresh();
                return;
            }

            setMode(Mode::None);
            return;
        }

        mStateDirty = false;

        switch (mMode)
        {
        case Mode::SendCurrentClosed:
        {
            if(before == AnyCircuitType::Closed && after == AnyCircuitType::Open)
            {
                setMode(Mode::SendCurrentWaitClosed);
            }
            break;
        }
        case Mode::ReceiveCurrentWaitClosed:
        case Mode::ReceiveCurrentClosed:
        {
            if(after == AnyCircuitType::Open && mFakeClosedCircuitsCount == 0)
            {
                // We do not have any fake/real closed circuit
                setMode(Mode::ReceiveCurrentOpen);
            }
        }
        default:
            break;
        }

        return;
    }
}

void RemoteCableCircuitNode::partialRemoveCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    AbstractCircuitNode::partialRemoveCircuit(circuit, items);

    if(circuit->type() == CircuitType::Open &&
            circuit->getSource() == this && circuit->getEnd() == this)
    {
        Q_ASSERT(mFakeClosedCircuitsCount > 0);
        mFakeClosedCircuitsCount--;
    }

    if(mFakeClosedCircuitsCount == 0 && !hasCircuits(CircuitType::Closed) &&
            (mMode == Mode::ReceiveCurrentWaitClosed || mMode == Mode::ReceiveCurrentClosed))
    {
        setMode(Mode::ReceiveCurrentOpen);
    }
}

bool RemoteCableCircuitNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    return true;
}

void RemoteCableCircuitNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);


}

QString RemoteCableCircuitNode::nodeType() const
{
    return NodeType;
}

bool RemoteCableCircuitNode::isSourceNode() const
{
    // Receive side acts as fake source controlled by send side
    return isReceiveSide();
}

bool RemoteCableCircuitNode::sourceDoNotCloseCircuits() const
{
    if(mMode == Mode::ReceiveCurrentOpen || mMode == Mode::ReceiveCurrentWaitClosed)
        return true;

    return false;
}

RemoteCableCircuitNode *RemoteCableCircuitNode::localPeer() const
{
    return mLocalPeer;
}

void RemoteCableCircuitNode::setLocalPeer(RemoteCableCircuitNode *newLocalPeer)
{
    if(mLocalPeer == newLocalPeer)
        return;

    if(newLocalPeer == this)
        return;

    if(mLocalPeer)
    {
        mLocalPeer->mLocalPeer = nullptr;
        mLocalPeer->onPeerModeChanged(Mode::None, mSendPole);
        emit mLocalPeer->peerChanged(mLocalPeer);

        mLocalPeer = nullptr;
    }

    mLocalPeer = newLocalPeer;

    if(mLocalPeer)
    {
        if(mLocalPeer->mLocalPeer)
        {
            mLocalPeer->mLocalPeer->mLocalPeer = nullptr;
            mLocalPeer->mLocalPeer->onPeerModeChanged(Mode::None, mSendPole);
            emit mLocalPeer->mLocalPeer->peerChanged(mLocalPeer);

            mLocalPeer->mLocalPeer = nullptr;
        }

        mLocalPeer->mLocalPeer = this;

        // Sync state
        if(isSendSide())
            mLocalPeer->onPeerModeChanged(mode(), mSendPole);
        else if(mLocalPeer->isSendSide())
            onPeerModeChanged(mLocalPeer->mode(), mSendPole);

        emit mLocalPeer->peerChanged(mLocalPeer);
    }
    else
    {
        // Allow only None and SendCurrentOpen
        if(mMode != Mode::SendCurrentOpen)
            setMode(Mode::None);
    }

    emit peerChanged(this);
}

RemoteCableCircuitNode::Mode RemoteCableCircuitNode::mode() const
{
    return mMode;
}

void RemoteCableCircuitNode::setMode(Mode newMode)
{
    if(mMode == newMode)
        return;

    const Mode oldMode = mMode;
    mMode = newMode;

    if(mMode == Mode::None || (mMode == Mode::SendCurrentOpen && oldMode != Mode::None))
    {
        // Disable previous circuits
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this);

        ElectricCircuit::defaultReachNextOpenCircuit(this);
    }
    else if(mMode == Mode::SendCurrentWaitClosed || mMode == Mode::SendCurrentClosed)
    {
        // Let open circuits pass trough our node
        // and see if it closes
        if(oldMode == Mode::SendCurrentOpen)
            ElectricCircuit::createCircuitsFromOtherNode(this);
    }
    else if(mMode == Mode::ReceiveCurrentOpen)
    {
        // Leave circuit open until we get response
        if(oldMode == Mode::None)
            ElectricCircuit::createCircuitsFromPowerNode(this, mRecvPole);
    }
    else if(mMode == Mode::ReceiveCurrentWaitClosed
            && oldMode == Mode::ReceiveCurrentClosed)
    {
        // Send side has closed circuits but now does not close anymore
        // go back to ReceiveCurrentWaitClosed state
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        CircuitList replacementCircuits;

        // Search all closed circuits
        for(ElectricCircuit *closed : closedCopy)
        {
            // This circuit now doesn't close on send side anymore
            // We morph it back into a fake open circuit
            ElectricCircuit *openClone = closed->cloneToOppositeType();
            if(openClone)
                replacementCircuits.append(openClone);
        }

        // Enable their replacments
        for(ElectricCircuit *open : std::as_const(replacementCircuits))
        {
            open->enableCircuit();
        }

        // Delete closed circuits
        disableCircuits(closedCopy, this);
    }
    else if(mMode == Mode::ReceiveCurrentClosed
            && oldMode == Mode::ReceiveCurrentWaitClosed)
    {
        const CircuitList openCopy = getCircuits(CircuitType::Open);
        CircuitList replacementCircuits;

        // Search all fake closed circuits
        for(ElectricCircuit *open : openCopy)
        {
            if(open->getSource() != this || open->getEnd() != this)
                continue;

            // This open circuit closes on receive side
            // Now that also send side has closes, morph into closed too
            ElectricCircuit *closedClone = open->cloneToOppositeType();
            if(closedClone)
                replacementCircuits.append(closedClone);
        }

        // Enable their replacments
        for(ElectricCircuit *closed : std::as_const(replacementCircuits))
        {
            closed->enableCircuit();
        }

        // Delete original fake circuits
        // Some of them might have been already deleted
        // so retrieve again the updated list
        CircuitList dummyList;
        const CircuitList openCopy2 = getCircuits(CircuitType::Open);
        for(ElectricCircuit *open : openCopy2)
        {
            if(open->getSource() != this || open->getEnd() != this)
                continue;

            open->terminateHere(open->getSource(), dummyList);
        }
    }

    // This function might recurse
    // In case mode changed in the meantime,
    // do not emit twice with old state
    if(mMode == newMode)
    {
        emit modeChanged(mMode);

        if(mLocalPeer)
        {
            // QMetaObject::invokeMethod(mLocalPeer,
            //                           &RemoteCableCircuitNode::onPeerModeChanged,
            //                           Qt::QueuedConnection,
            //                           mMode, mSendPole);

            // Simulate network latency for when it will transmit to other PC
            const Mode currMode = mMode;
            const CircuitPole currSendPole = mSendPole;
            QTimer::singleShot(200, Qt::VeryCoarseTimer,
                               this, [this, currMode, currSendPole]()
            {
                if(mLocalPeer)
                {
                    mLocalPeer->onPeerModeChanged(currMode, currSendPole);
                }
            });
        }
    }
}

void RemoteCableCircuitNode::onPeerModeChanged(Mode peerMode, CircuitPole peerSendPole)
{
    mRecvPole = peerSendPole;

    switch (peerMode)
    {
    case Mode::None:
    {
        if(isReceiveSide())
        {
            setMode(Mode::None);
        }
        else if(isSendSide())
        {
            setMode(Mode::SendCurrentOpen);
        }
        break;
    }
    case Mode::SendCurrentOpen:
    {
        if(isSendSide())
        {
            // Cannot have 2 sources, reject change
            setMode(Mode::None);
        }
        else
        {
            if(mMode == Mode::None || mMode == Mode::ReceiveCurrentClosed)
                setMode(Mode::ReceiveCurrentOpen);
        }
        break;
    }
    case Mode::SendCurrentWaitClosed:
    {
        if(mMode == Mode::ReceiveCurrentClosed)
        {
            setMode(Mode::ReceiveCurrentWaitClosed);
        }
        else if(mMode != Mode::ReceiveCurrentWaitClosed)
        {
            // Reject change
            setMode(Mode::None);
        }
        break;
    }
    case Mode::SendCurrentClosed:
    {
        if(mMode != Mode::ReceiveCurrentWaitClosed
                && mMode != Mode::ReceiveCurrentClosed)
        {
            // Reject change
            setMode(Mode::None);
        }

        setMode(Mode::ReceiveCurrentClosed);
        break;
    }
    case Mode::ReceiveCurrentOpen:
    {
        if(!isSendSide())
        {
            // Reject change
            setMode(Mode::None);
        }
        else
        {
            setMode(Mode::SendCurrentOpen);
        }
        break;
    }
    case Mode::ReceiveCurrentWaitClosed:
    {
        if(mMode != Mode::SendCurrentOpen && mMode != Mode::SendCurrentWaitClosed)
        {
            // Reject change
            setMode(Mode::None);
        }
        else
        {
            setMode(Mode::SendCurrentWaitClosed);
        }
        break;
    }
    case Mode::ReceiveCurrentClosed:
    {
        if(mMode != Mode::SendCurrentClosed)
        {
            // Reject change
            setMode(Mode::None);
        }
        break;
    }
    default:
        break;
    }

    mLastPeerMode = peerMode;
}

void RemoteCableCircuitNode::scheduleStateRefresh()
{
    if(mStateDirty)
        return; // Already scheduled

    mStateDirty = true;
    QMetaObject::invokeMethod(this, &RemoteCableCircuitNode::refreshState,
                              Qt::QueuedConnection);
}

void RemoteCableCircuitNode::refreshState()
{
    if(!mStateDirty)
        return; // Already refreshed in the meantime

    if(hasAnyCircuit(0) == AnyCircuitType::None)
    {
        setMode(Mode::None);
    }

    mStateDirty = false;
}
