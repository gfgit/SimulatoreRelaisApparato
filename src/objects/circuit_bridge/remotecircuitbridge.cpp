/**
 * src/objects/circuit_bridge/remotecircuitbridge.cpp
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

#include "remotecircuitbridge.h"

#include "../abstractsimulationobjectmodel.h"

#include "../../circuits/nodes/remotecablecircuitnode.h"

#include "../../views/modemanager.h"

#include "../../network/remotemanager.h"
#include "../../network/remotesession.h"

#include "../../serial/serialmanager.h"
#include "../../serial/serialdevice.h"

#include <QTimer>

#include <QJsonObject>

RemoteCircuitBridge::RemoteCircuitBridge(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{
    connect(this, &AbstractSimulationObject::nameChanged,
            this, &RemoteCircuitBridge::onNameChanged);
}

RemoteCircuitBridge::~RemoteCircuitBridge()
{
    disconnect(this, &AbstractSimulationObject::nameChanged,
               this, &RemoteCircuitBridge::onNameChanged);

    setIsRemote(false);

    if(mNodeA)
    {
        mNodeA->setRemote(nullptr);
        mNodeA = nullptr;
    }

    if(mNodeB)
    {
        mNodeB->setRemote(nullptr);
        mNodeB = nullptr;
    }
}

QString RemoteCircuitBridge::getType() const
{
    return Type;
}

bool RemoteCircuitBridge::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true;

    mNodeDescriptionA = obj.value("node_descr_A").toString().trimmed();
    if(mNodeDescriptionA == name())
        mNodeDescriptionA.clear();

    mNodeDescriptionB = obj.value("node_descr_B").toString().trimmed();
    if(mNodeDescriptionB == name())
        mNodeDescriptionB.clear();

    setPeerNodeCustomName(obj.value("remote_custom_node").toString());
    const QString peerSessionName = obj.value("remote_session").toString().trimmed();
    if(!peerSessionName.isEmpty())
    {
        RemoteManager *remoteMgr = model()->modeMgr()->getRemoteManager();
        RemoteSession *remoteSession = remoteMgr->addRemoteSession(peerSessionName);
        setRemoteSession(remoteSession);
    }

    mSerialInputId = obj.value("device_input_id").toInt();
    mSerialOutputId = obj.value("device_output_id").toInt();

    const QString devName = obj.value("device_name").toString().simplified();
    if(!devName.isEmpty())
    {
        SerialManager *serialMgr = model()->modeMgr()->getSerialManager();
        SerialDevice *dev = serialMgr->addDevice(devName);
        setSerialDevice(dev);
    }

    return true;
}

void RemoteCircuitBridge::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["node_descr_A"] = mNodeDescriptionA != name() ? mNodeDescriptionA : QString();
    obj["node_descr_B"] = mNodeDescriptionB != name() ? mNodeDescriptionB : QString();

    obj["remote_session"] = mRemoteSession ? mRemoteSession->getSessionName() : QString();
    obj["remote_custom_node"] = mPeerNodeCustomName;

    obj["device_name"] = mSerialDevice ? mSerialDevice->getName() : QString();
    obj["device_input_id"] = mSerialInputId;
    obj["device_output_id"] = mSerialOutputId;
}

int RemoteCircuitBridge::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int nodesCount = AbstractSimulationObject::getReferencingNodes(result);

    if(mNodeA)
    {
        nodesCount++;
        if(result)
            result->append(mNodeA);
    }

    if(mNodeB)
    {
        nodesCount++;
        if(result)
            result->append(mNodeB);
    }

    return nodesCount;
}

RemoteCableCircuitNode *RemoteCircuitBridge::getNode(bool isA) const
{
    return isA ? mNodeA : mNodeB;
}

void RemoteCircuitBridge::setNodeDescription(bool isA, const QString &newDescr)
{
    if(newDescr == getNodeDescription(isA))
        return;

    QString &target = isA ? mNodeDescriptionA : mNodeDescriptionB;
    target = newDescr;

    // Trigger node update
    RemoteCableCircuitNode *targetNode = getNode(isA);
    if(targetNode)
        emit targetNode->shapeChanged();

    emit settingsChanged(this);
}

bool RemoteCircuitBridge::isRemote() const
{
    if(mRemoteSession || mSerialDevice)
        return true;

    return false;
}

QString RemoteCircuitBridge::remoteSessionName() const
{
    if(mRemoteSession)
        return mRemoteSession->getSessionName();
    return QString();
}

bool RemoteCircuitBridge::setRemoteSession(RemoteSession *remoteSession)
{
    if(mRemoteSession == remoteSession)
        return true;

    if(remoteSession)
    {
        if(mNodeA && mNodeB)
            return false;

        if(!remoteSession->isRemoteBridgeNameAvailable(peerNodeName()))
            return false;
    }

    if(mRemoteSession)
        mRemoteSession->removeRemoteBridge(this);

    mRemoteSession = remoteSession;

    if(remoteSession)
    {
        remoteSession->addRemoteBridge(this);

        setIsRemote(true);
    }

    emit settingsChanged(this);
    return true;
}

void RemoteCircuitBridge::setIsRemote(bool val)
{
    if(val && !mNodeA && mNodeB)
    {
        // Swap nodes and descriptions
        qSwap(mNodeDescriptionA, mNodeDescriptionB);
        mNodeB->setIsNodeA(true);
    }
    else if(!val)
    {
        setRemoteSession(nullptr);
        setSerialDevice(nullptr);
    }
}

void RemoteCircuitBridge::setNode(RemoteCableCircuitNode *newNode, bool isA)
{
    if(newNode && (mNodeA == newNode || mNodeB == newNode))
        return;

    RemoteCableCircuitNode *&target = isA ? mNodeA : mNodeB;
    if(newNode == target)
        return;

    RemoteCableCircuitNode *other = getNode(!isA);

    if(target)
        target->setRemote(nullptr);

    target = newNode;

    if(other)
    {
        if(target)
        {
            if(target->isSendSide())
                onLocalNodeModeChanged(target);
            else if(other->isSendSide())
                onLocalNodeModeChanged(other);
        }
        else
        {
            other->setMode(RemoteCableCircuitNode::Mode::None);
        }
    }
    else if(target)
    {
        target->setMode(RemoteCableCircuitNode::Mode::None);
    }

    if(mNodeA && mNodeB)
        setIsRemote(false);

    emit nodesChanged(this);
    emit settingsChanged(this);
}

void RemoteCircuitBridge::onLocalNodeModeChanged(RemoteCableCircuitNode *node)
{
    node->flagsNeedUpdate = false;

    RemoteCableCircuitNode *other = node == mNodeA ? mNodeB : mNodeA;

    const RemoteCableCircuitNode::Mode currMode = node->mode();
    const CircuitPole currSendPole = node->getSendPole();
    const RemoteCableCircuitNode::Mode replyToMode = node->lastPeerMode();
    const CircuitFlags circuitFlags = node->getCircuitFlags(0);
    const CircuitFlags nonSourceFlags = node->getNonSourceFlags();

    CircuitFlags flagsToSend = circuitFlags;
    if(RemoteCableCircuitNode::isReceiveMode(currMode))
        flagsToSend = nonSourceFlags;

    if(other)
    {
        // NOTE: we cannot directly call onPeerModeChanged()
        // because it triggers circuit add/remove from inside
        // another circuit add/remove.
        // So use delayed event posting
        other->delayedPeerModeChanged(currMode, currSendPole,
                                      replyToMode, flagsToSend);
    }
    else if(mRemoteSession && mPeerNodeId)
    {
        // Send to remote session
        mRemoteSession->onLocalBridgeModeChanged(mPeerNodeId,
                                                 qint8(currMode), qint8(currSendPole),
                                                 qint8(replyToMode), quint8(flagsToSend));
    }
    else if(mSerialNameId)
    {
        // Send to serial device
        if(RemoteCableCircuitNode::isSendMode(currMode) && mSerialOutputId)
        {
            if(currMode == RemoteCableCircuitNode::Mode::SendCurrentOpen)
            {
                // Fake close circuit
                node->delayedPeerModeChanged(RemoteCableCircuitNode::Mode::ReceiveCurrentWaitClosed,
                                             currSendPole,
                                             RemoteCableCircuitNode::Mode::ReceiveCurrentWaitClosed,
                                             CircuitFlags::None);
            }
            else if(currMode == RemoteCableCircuitNode::Mode::SendCurrentClosed)
            {
                // Fake close circuit
                node->delayedPeerModeChanged(RemoteCableCircuitNode::Mode::ReceiveCurrentClosed,
                                             currSendPole,
                                             RemoteCableCircuitNode::Mode::ReceiveCurrentClosed,
                                             CircuitFlags::None);

                int mode = currSendPole == CircuitPole::First ? 1 : 2;
                mSerialDevice->onOutputChanged(mSerialOutputId, mode);
            }
        }
        else if(currMode == RemoteCableCircuitNode::Mode::None && mSerialOutputId)
        {
            int mode = 0;
            mSerialDevice->onOutputChanged(mSerialOutputId, mode);

            // Fake reset circuit
            node->delayedPeerModeChanged(RemoteCableCircuitNode::Mode::None,
                                         currSendPole,
                                         RemoteCableCircuitNode::Mode::None,
                                         CircuitFlags::None);
        }
        else if(currMode == RemoteCableCircuitNode::Mode::ReceiveCurrentWaitClosed && mSerialInputId)
        {
            // Fake close remote circuit
            node->delayedPeerModeChanged(RemoteCableCircuitNode::Mode::SendCurrentClosed,
                                         node->mRecvPole,
                                         RemoteCableCircuitNode::Mode::SendCurrentClosed,
                                         circuitFlags);
        }
    }
}

void RemoteCircuitBridge::onSerialInputMode(int mode)
{
    const CircuitFlags circuitFlags = mNodeA->getCircuitFlags(0);

    if(mode == 1 || mode == 2)
    {
        // Enable input
        if(mNodeA->mode() == RemoteCableCircuitNode::Mode::None)
        {
            mNodeA->onPeerModeChanged(RemoteCableCircuitNode::Mode::SendCurrentOpen,
                                      mode == 1 ? CircuitPole::First : CircuitPole::Second,
                                      circuitFlags);
        }
    }
    else
    {
        // Disable input
        mNodeA->onPeerModeChanged(RemoteCableCircuitNode::Mode::None,
                                  CircuitPole::First,
                                  circuitFlags);
    }
}

int RemoteCircuitBridge::serialInputId() const
{
    return mSerialInputId;
}

void RemoteCircuitBridge::setSerialInputId(int newSerialInputId)
{
    if(mSerialInputId == newSerialInputId)
        return;

    if(mSerialDevice)
    {
        bool ret = mSerialDevice->changeRemoteBridgeInput(this,
                                                          mSerialInputId,
                                                          newSerialInputId);
        if(!ret)
            return;
    }

    mSerialInputId = newSerialInputId;
    emit settingsChanged(this);
}

int RemoteCircuitBridge::serialOutputId() const
{
    return mSerialOutputId;
}

void RemoteCircuitBridge::setSerialOutputId(int newSerialOutputId)
{
    if(mSerialOutputId == newSerialOutputId)
        return;

    if(mSerialDevice)
    {
        bool ret = mSerialDevice->changeRemoteBridgeOutput(this,
                                                           mSerialOutputId,
                                                           newSerialOutputId);
        if(!ret)
            return;
    }

    mSerialOutputId = newSerialOutputId;
    emit settingsChanged(this);
}

void RemoteCircuitBridge::onNameChanged(AbstractSimulationObject *,
                                        const QString &newName, const QString &oldName)
{
    if(!mPeerNodeCustomName.isEmpty())
        return;

    if(!mRemoteSession || oldName.isEmpty())
        return;

    if(!mRemoteSession->isRemoteBridgeNameAvailable(newName, this))
        setPeerNodeCustomName(oldName);
}

QString RemoteCircuitBridge::peerNodeName() const
{
    return mPeerNodeCustomName.isEmpty() ? name() : mPeerNodeCustomName;
}

bool RemoteCircuitBridge::setPeerNodeCustomName(const QString &newPeerNodeName)
{
    const QString nameTrimmed = newPeerNodeName.trimmed();

    if(mPeerNodeCustomName == nameTrimmed)
        return true;

    if(mRemoteSession)
    {
        const QString newPeerName = nameTrimmed.isEmpty() ? name() : nameTrimmed;
        if(!mRemoteSession->isRemoteBridgeNameAvailable(newPeerName))
            return false;
    }

    mPeerNodeCustomName = nameTrimmed;

    emit settingsChanged(this);
    return true;
}

QString RemoteCircuitBridge::getSerialDeviceName() const
{
    return mSerialDevice ? mSerialDevice->getName() : QString();
}

bool RemoteCircuitBridge::isRemoteSessionConnected() const
{
    return mRemoteSession && mRemoteSession->getConnection()
            && mPeerNodeId != 0;
}

bool RemoteCircuitBridge::setSerialDevice(SerialDevice *serialDevice)
{
    if(mSerialDevice == serialDevice)
        return true;

    if(serialDevice)
    {
        if(mNodeA && mNodeB)
            return false;

        if(mSerialInputId &&
                !serialDevice->isInputFree(mSerialInputId))
            return false;

        if(mSerialOutputId &&
                !serialDevice->isOutputFree(mSerialOutputId))
            return false;
    }

    if(mSerialDevice)
        mSerialDevice->removeRemoteBridge(this);

    mSerialDevice = serialDevice;

    if(serialDevice)
    {
        serialDevice->addRemoteBridge(this);

        setIsRemote(true);
    }

    emit settingsChanged(this);
    return true;
}

void RemoteCircuitBridge::onRemoteNodeModeChanged(qint8 mode, qint8 pole,
                                                  qint8 replyToMode, quint8 circuitFlags)
{
    const RemoteCableCircuitNode::Mode currMode = RemoteCableCircuitNode::Mode(mode);
    const CircuitPole currSendPole = CircuitPole(pole);
    const RemoteCableCircuitNode::Mode replyMode = RemoteCableCircuitNode::Mode(replyToMode);
    const CircuitFlags recvFlags = CircuitFlags(circuitFlags);

    if(!mNodeA)
        return;

    // NOTE: If 2 requests are quickly sent on after another,
    // we might get response to first one after we already sent the second one
    // so here we ignore old replies by comparing which mode generated them with
    // our current state
    if(mNodeA->mode() != replyMode &&
            RemoteCableCircuitNode::isReceiveMode(currMode))
        return;

    mNodeA->onPeerModeChanged(currMode, currSendPole, recvFlags);
}

void RemoteCircuitBridge::onRemoteDisconnected()
{
    if(mNodeA)
        mNodeA->onPeerModeChanged(RemoteCableCircuitNode::Mode::None,
                                  CircuitPole::First,
                                  CircuitFlags::None);
}

void RemoteCircuitBridge::onRemoteStarted()
{
    if(!mNodeA)
        return;

    const RemoteCableCircuitNode::Mode currMode = mNodeA->mode();
    const CircuitPole currSendPole = mNodeA->getSendPole();
    const RemoteCableCircuitNode::Mode replyToMode = RemoteCableCircuitNode::Mode::None;
    const CircuitFlags circuitFlags = mNodeA->getCircuitFlags(0);

    if(!RemoteCableCircuitNode::isSendMode(currMode))
        return;

    if(mRemoteSession && mPeerNodeId)
    {
        // Send to remote session
        mRemoteSession->onLocalBridgeModeChanged(mPeerNodeId,
                                                 qint8(currMode), qint8(currSendPole),
                                                 qint8(replyToMode), quint8(circuitFlags));
    }
    else if(mSerialNameId && mSerialOutputId)
    {
        if(currMode == RemoteCableCircuitNode::Mode::SendCurrentOpen)
        {
            // Fake close circuit, this will then send to serial device
            mNodeA->onPeerModeChanged(RemoteCableCircuitNode::Mode::ReceiveCurrentWaitClosed,
                                      currSendPole,
                                      CircuitFlags::None);
        }
    }
}
