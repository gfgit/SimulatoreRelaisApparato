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

#include "../../objects/abstractsimulationobjectmodel.h"
#include "../../objects/circuit_bridge/remotecircuitbridge.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

#include <QScopedValueRollback>

#include <QCoreApplication> // for postEvent()

class RemoteCableDelayedPeerMode : public QEvent
{
public:
    static const QEvent::Type _Type = QEvent::Type(QEvent::User + 2);

    RemoteCableDelayedPeerMode(RemoteCableCircuitNode::Mode peerMode, CircuitPole peerSendPole)
        : QEvent(_Type)
        , mPeerMode(peerMode)
        , mPeerSendPole(peerSendPole)
    {};

    inline RemoteCableCircuitNode::Mode peerMode() const { return mPeerMode; }
    inline CircuitPole peerSendPole() const { return mPeerSendPole; }

private:
    RemoteCableCircuitNode::Mode mPeerMode;
    CircuitPole mPeerSendPole;
};

RemoteCableCircuitNode::RemoteCableCircuitNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));
}

RemoteCableCircuitNode::~RemoteCableCircuitNode()
{
    setRemote(nullptr);
}

bool RemoteCableCircuitNode::event(QEvent *e)
{
    if(e->type() == RemoteCableDelayedPeerMode::_Type)
    {
        RemoteCableDelayedPeerMode *ev = static_cast<RemoteCableDelayedPeerMode *>(e);
        onPeerModeChanged(ev->peerMode(), ev->peerSendPole());
        return true;
    }

    return AbstractCircuitNode::event(e);
}

AbstractCircuitNode::ConnectionsRes RemoteCableCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0)
        return {};

    if(mMode != Mode::SendCurrentWaitClosed &&
            mMode != Mode::SendCurrentClosed)
        return {}; // Leave circuit open

    // Close the circuit
    CableItemFlags dest;
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
            && circuit->isDifferentPoleStartEnd()
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
                mSendPole = items.first().fromPole();

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
    // Set insideRemoveCircuit to true and reset to false on function return
    QScopedValueRollback<bool> guard(insideRemoveCircuit, true);

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
            circuit->getSource() == this && circuit->getEnd() == this
            && circuit->isDifferentPoleStartEnd())
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

    auto model = modeMgr()->modelForType(RemoteCircuitBridge::Type);
    if(model)
    {
        const QString remoteName = obj.value("remote").toString();
        AbstractSimulationObject *remoteObj = model->getObjectByName(remoteName);

        mIsNodeA = obj.value("node_A").toBool(true);

        // Do not auto swap based on remote nodes.
        // We do it only for newly created items during editing
        setRemote(static_cast<RemoteCircuitBridge *>(remoteObj), false, false);
    }
    else
        setRemote(nullptr);

    return true;
}

void RemoteCableCircuitNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["remote"] = mRemote ? mRemote->name() : QString();
    obj["node_A"] = mIsNodeA;
}

QString RemoteCableCircuitNode::nodeType() const
{
    return NodeType;
}

bool RemoteCableCircuitNode::isSourceNode(bool onlyCurrentState, int nodeContact) const
{
    if(nodeContact != 0 && nodeContact != NodeItem::InvalidContact)
        return false;

    if(!onlyCurrentState)
        return true;

    // Receive side acts as fake source controlled by send side
    return isReceiveSide();
}

bool RemoteCableCircuitNode::sourceDoNotCloseCircuits() const
{
    if(mMode == Mode::ReceiveCurrentOpen || mMode == Mode::ReceiveCurrentWaitClosed)
        return true;

    return false;
}

bool RemoteCableCircuitNode::isSourceEnabled() const
{
    return mIsEnabled;
}

void RemoteCableCircuitNode::setSourceEnabled(bool newEnabled)
{
    if(modeMgr()->mode() == FileMode::Editing && newEnabled)
        return; // Prevent enabling during editing

    if (mIsEnabled == newEnabled)
        return;
    mIsEnabled = newEnabled;

    if(mIsEnabled)
    {
        const AnyCircuitType after = hasAnyCircuit(0);

        if(after == AnyCircuitType::Open)
        {
            setMode(Mode::SendCurrentOpen);
        }
    }
    else
    {
        // Disable circuits
        setMode(Mode::None);
    }
}

void RemoteCableCircuitNode::setMode(Mode newMode)
{
    if(!mIsEnabled)
    {
        if(isReceiveMode(newMode))
            newMode = Mode::None; // Cannot receive if not enabled
        else if(newMode == Mode::SendCurrentWaitClosed || newMode == Mode::SendCurrentClosed)
            newMode = Mode::SendCurrentOpen; // Cannot close if not enabled
    }

    if(mMode == newMode)
        return;

    const Mode oldMode = mMode;
    mMode = newMode;

    if(mMode == Mode::None || (mMode == Mode::SendCurrentOpen && oldMode != Mode::None))
    {
        if(insideRemoveCircuit)
        {
            // Cannot disable circuits if inside removeCircuit()
            scheduleStateRefresh();
        }
        else
        {
            // Disable previous circuits
            const CircuitList closedCopy = getCircuits(CircuitType::Closed);
            disableCircuits(closedCopy, this);

            const CircuitList openCopy = getCircuits(CircuitType::Open);
            truncateCircuits(openCopy, this);

            ElectricCircuit::defaultReachNextOpenCircuit(this);
        }
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
            if(open->getSource() != this || open->getEnd() != this
                    || !open->isDifferentPoleStartEnd())
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
            if(open->getSource() != this || open->getEnd() != this
                    || !open->isDifferentPoleStartEnd())
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

        if(mRemote)
        {
            mRemote->onLocalNodeModeChanged(this);
        }
    }
}

void RemoteCableCircuitNode::onPeerModeChanged(Mode peerMode, CircuitPole peerSendPole)
{
    mRecvPole = peerSendPole;
    mLastPeerMode = peerMode;

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
        if(mMode != Mode::SendCurrentOpen &&
                mMode != Mode::SendCurrentWaitClosed &&
                mMode != Mode::SendCurrentClosed)
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
}

void RemoteCableCircuitNode::delayedPeerModeChanged(Mode peerMode, CircuitPole peerSendPole)
{
    QCoreApplication::postEvent(this, new RemoteCableDelayedPeerMode(peerMode, peerSendPole));
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

    if(mMode == Mode::None || mMode == Mode::SendCurrentOpen)
    {
        // Disable previous circuits, out of removeCircuit()
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this);

        ElectricCircuit::defaultReachNextOpenCircuit(this);
    }

    mStateDirty = false;
}

bool RemoteCableCircuitNode::isNodeA() const
{
    return mIsNodeA;
}

void RemoteCableCircuitNode::setIsNodeA(bool newIsNodeA)
{
    if(mIsNodeA == newIsNodeA)
        return;

    if(mRemote)
    {
        // Swap nodes
        RemoteCableCircuitNode *other = mRemote->getNode(!mIsNodeA);
        mRemote->setNode(nullptr, mIsNodeA);
        mRemote->setNode(nullptr, !mIsNodeA);

        mRemote->setNode(this, newIsNodeA);
        if(other)
        {
            // We call setIsNodeA() while remote is still not set
            // to avoid recursion
            other->setIsNodeA(!newIsNodeA);
            mRemote->setNode(other, !newIsNodeA);
        }
    }

    mIsNodeA = newIsNodeA;

    emit shapeChanged();
    modeMgr()->setFileEdited();
}

QString RemoteCableCircuitNode::getDescription() const
{
    if(!mRemote)
        return tr("BRIDGE!!!");

    // We show description of our local node
    QString str = mRemote->getNodeDescription(mIsNodeA);
    if(str.isEmpty())
        return tr("EMPTY!!!");

    return str;
}

RemoteCircuitBridge *RemoteCableCircuitNode::remote() const
{
    return mRemote;
}

bool RemoteCableCircuitNode::setRemote(RemoteCircuitBridge *newRemote, bool autoSwap, bool force)
{
    if(mRemote == newRemote)
        return true;

    if(!force && newRemote)
    {
        const bool hasA = newRemote->getNode(true);
        const bool hasB = newRemote->getNode(false);

        if(hasA && hasB)
            return false;

        if(newRemote->isRemote() && (hasA || hasB))
            return false;
    }

    if(mRemote)
    {
        // RemoteCircuitBridge::setNode() can call us again
        // So we reset mRemote first and call it just after.
        // This way next setRemote() call will return on first if line.
        RemoteCircuitBridge *oldRemote = mRemote;
        mRemote = nullptr;
        oldRemote->setNode(nullptr, mIsNodeA);
    }

    mRemote = newRemote;

    if(mRemote)
    {
        // Try to set where there is no node set
        if(autoSwap && !mRemote->getNode(!mIsNodeA))
        {
            if(mRemote->getNode(mIsNodeA))
            {
                // Our slot is taken, other is free, swap
                mIsNodeA = !mIsNodeA;
            }
            else
            {
                // Both slots are free, use A first
                mIsNodeA = true;
            }
        }

        mRemote->setNode(this, mIsNodeA);
    }
    else
    {
        setMode(Mode::None);
    }

    emit shapeChanged();
    modeMgr()->setFileEdited();

    return true;
}
