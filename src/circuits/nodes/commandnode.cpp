/**
 * src/circuits/nodes/commandnode.cpp
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

#include "commandnode.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"
#include "../../objects/simulationobjectfactory.h"

#include "../../objects/interfaces/buttoninterface.h"
#include "../../objects/interfaces/leverinterface.h"

#include <QTimerEvent>

#include <QJsonObject>

CommandNode::CommandNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));
}

CommandNode::~CommandNode()
{
    mTimer.stop();
    setObject(nullptr);
}

QString CommandNode::nodeType() const
{
    return NodeType;
}

AbstractCircuitNode::ConnectionsRes CommandNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 || !mContacts.at(0).cable)
        return {};

    // Close the circuit
    CableItemFlags dest;
    dest.cable.cable = mContacts.at(0).cable;
    dest.cable.side = mContacts.at(0).cableSide;
    dest.nodeContact = 0;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void CommandNode::addCircuit(ElectricCircuit *circuit)
{
    const bool wasActive = hasCircuits();

    AbstractCircuitNode::addCircuit(circuit);

    const bool isActive = hasCircuits();

    if(!wasActive && isActive && mObject)
    {
        mTimer.start(std::chrono::milliseconds(mDelayMillis),
                     Qt::CoarseTimer, this);
    }
}

void CommandNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const bool wasActive = hasCircuits();

    AbstractCircuitNode::removeCircuit(circuit, items);

    const bool isActive = hasCircuits();

    if(wasActive && !isActive && mObject)
    {
        mTimer.stop();
    }
}

bool CommandNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    const QString objType = obj.value("object_type").toString();
    auto model = modeMgr()->modelForType(objType);
    if(model)
    {
        const QString objName = obj.value("object").toString();
        AbstractSimulationObject *commandedObj = model->getObjectByName(objName);
        setObject(commandedObj);
    }
    else
        setObject(nullptr);

    setDelayMillis(obj.value("delay_ms").toInt(500));
    setTargetPosition(obj.value("target_pos").toInt(0));

    return true;
}

void CommandNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["object"] = mObject ? mObject->name() : QString();
    obj["object_type"] = mObject ? mObject->getType() : QString();
    obj["delay_ms"] = mDelayMillis;
    obj["target_pos"] = mTargetPosition;
}

void CommandNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty objProp;
    objProp.name = "object";
    objProp.prettyName = tr("Target");
    objProp.types = supportedObjectTypes();

    result.append(objProp);
}

int CommandNode::delayMillis() const
{
    return mDelayMillis;
}

void CommandNode::setDelayMillis(int newDelayMillis)
{
    newDelayMillis = std::clamp(newDelayMillis, 10, 2000);
    if(mDelayMillis == newDelayMillis)
        return;

    mDelayMillis = newDelayMillis;
    emit shapeChanged(false);
    modeMgr()->setFileEdited();
}

void CommandNode::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == mTimer.timerId())
    {
        performAction();
        mTimer.stop();
        return;
    }

    AbstractCircuitNode::timerEvent(ev);
}

void CommandNode::performAction()
{
    if(!mObject)
        return;

    if(LeverInterface *leverIface = mObject->getInterface<LeverInterface>())
    {
        leverIface->setAngle(leverIface->angleForPosition(mTargetPosition));
        leverIface->setPosition(mTargetPosition);
    }
    else if(ButtonInterface *butIface = mObject->getInterface<ButtonInterface>())
    {
        ButtonInterface::State s = ButtonInterface::State(mTargetPosition);
        switch (s)
        {
        case ButtonInterface::State::Pressed:
        case ButtonInterface::State::Normal:
        case ButtonInterface::State::Extracted:
            butIface->setState(s);
            break;
        default:
            break;
        }
    }
}

int CommandNode::targetPosition() const
{
    return mTargetPosition;
}

void CommandNode::setTargetPosition(int newTargetPosition)
{
    EnumDesc desc;
    if(getObjectPosDesc(desc))
    {
        newTargetPosition = std::clamp(newTargetPosition,
                                       desc.minValue,
                                       desc.maxValue);
    }

    if(mTargetPosition == newTargetPosition)
        return;

    mTargetPosition = newTargetPosition;

    emit shapeChanged(false);
    modeMgr()->setFileEdited();
}

bool CommandNode::getObjectPosDesc(EnumDesc &descOut) const
{
    if(!mObject)
        return false;

    if(LeverInterface *leverIface = mObject->getInterface<LeverInterface>())
    {
        descOut = leverIface->positionDesc();
        return true;
    }
    else if(ButtonInterface *butIface = mObject->getInterface<ButtonInterface>())
    {
        descOut = ButtonInterface::getStateDesc();
        return true;
    }

    return false;
}

QStringList CommandNode::supportedObjectTypes() const
{
    SimulationObjectFactory *factory = modeMgr()->objectFactory();

    QStringList types;
    types.append(factory->typesForInterface(ButtonInterface::IfaceType));
    types.append(factory->typesForInterface(LeverInterface::IfaceType));
    types.removeDuplicates();
    return types;
}

AbstractSimulationObject *CommandNode::object() const
{
    return mObject;
}

void CommandNode::setObject(AbstractSimulationObject *newObject)
{
    if(mObject == newObject)
        return;

    if(newObject && !newObject->getInterface<ButtonInterface>() && !newObject->getInterface<LeverInterface>())
        return;

    mTimer.stop();
    mObject = newObject;

    // Ensure target position is still in range
    setTargetPosition(targetPosition());

    emit shapeChanged(false);
    emit objectChanged(mObject);
    modeMgr()->setFileEdited();
}
