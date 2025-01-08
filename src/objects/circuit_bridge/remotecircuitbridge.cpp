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

#include "../../circuits/nodes/remotecablecircuitnode.h"

#include <QTimer>

#include <QJsonObject>

RemoteCircuitBridge::RemoteCircuitBridge(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{

}

RemoteCircuitBridge::~RemoteCircuitBridge()
{
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

    mNodeDescriptionA = obj.value("node_descr_A").toString();
    mNodeDescriptionB = obj.value("node_descr_B").toString();

    return true;
}

void RemoteCircuitBridge::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["node_descr_A"] = mNodeDescriptionA;
    obj["node_descr_B"] = mNodeDescriptionB;
}

QVector<AbstractCircuitNode *> RemoteCircuitBridge::nodes() const
{
    QVector<AbstractCircuitNode *> result;
    result.reserve(2);

    if(mNodeA)
        result.append(mNodeA);

    if(mNodeB)
        result.append(mNodeB);

    return result;
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

    // Trigger opposite node update
    RemoteCableCircuitNode *other = getNode(!isA);
    if(other)
        emit other->shapeChanged();

    emit settingsChanged(this);
}

void RemoteCircuitBridge::setNode(RemoteCableCircuitNode *newNode, bool isA)
{
    if(mNodeA == newNode || mNodeB == newNode)
        return;

    RemoteCableCircuitNode *&target = isA ? mNodeA : mNodeB;
    RemoteCableCircuitNode *other = getNode(!isA);

    if(target)
        target->setRemote(nullptr);

    target = newNode;

    if(other)
    {
        if(target)
        {
            if(target->isSendSide())
                onNodeModeChanged(target);
            else if(other->isSendSide())
                onNodeModeChanged(other);
        }
        else
        {
            other->setMode(RemoteCableCircuitNode::Mode::None);
        }
    }
    else
    {
        target->setMode(RemoteCableCircuitNode::Mode::None);
    }

    emit settingsChanged(this);
}

void RemoteCircuitBridge::onNodeModeChanged(RemoteCableCircuitNode *node)
{
    RemoteCableCircuitNode *other = node == mNodeA ? mNodeB : mNodeA;
    if(!other)
        return;

    // NOTE: we cannot directly call onPeerModeChanged()
    // because it triggers circuit add/remove from inside
    // another circuit add/remove.
    // So use Qt::QueuedConnection

    // QMetaObject::invokeMethod(mLocalPeer,
    //                           &RemoteCableCircuitNode::onPeerModeChanged,
    //                           Qt::QueuedConnection,
    //                           mMode, mSendPole);

    // Simulate network latency for when it will transmit to other PC
    const RemoteCableCircuitNode::Mode currMode = node->mode();
    const CircuitPole currSendPole = node->getSendPole();

    QTimer::singleShot(200, Qt::VeryCoarseTimer,
                       this, [this, currMode, currSendPole, other]()
    {
        if(other != mNodeA && other != mNodeB)
            return; // Node was deleted/unlinked

        other->onPeerModeChanged(currMode, currSendPole);
    });
}
