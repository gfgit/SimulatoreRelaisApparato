/**
 * src/circuits/nodes/lightbulbnode.cpp
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

#include "lightbulbnode.h"

#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../views/modemanager.h"

LightBulbNode::LightBulbNode(ModeManager *mgr, QObject *parent)
    : SimpleActivationNode{mgr, parent}
{

}

QString LightBulbNode::nodeType() const
{
    return NodeType;
}

QString LightBulbNode::allowedObjectType() const
{
    return LightBulbObject::Type;
}

void LightBulbNode::onCircuitFlagsChanged()
{
    const CircuitFlags flags = getCircuitFlags(0);

    switch (getCode(flags))
    {
    case CircuitFlags::None:
    case CircuitFlags::CodeInvalid:
    default:
    {
        disconnect(modeMgr(), &ModeManager::codeTimerChanged,
                   this, &LightBulbNode::onCodePhaseChanged);
        break;
    }
    case CircuitFlags::Code75:
    case CircuitFlags::Code120:
    case CircuitFlags::Code180:
    case CircuitFlags::Code270:
    {
        connect(modeMgr(), &ModeManager::codeTimerChanged,
                this, &LightBulbNode::onCodePhaseChanged,
                Qt::UniqueConnection);
        break;
    }
    }

    onCodePhaseChanged();
}

void LightBulbNode::onCodePhaseChanged()
{
    if(!object() || !hasCircuits(CircuitType::Closed))
    {
        setNodeState(false);
        return;
    }

    const CircuitFlags flags = getCircuitFlags(0);

    switch (getCode(flags))
    {
    case CircuitFlags::None:
    case CircuitFlags::CodeInvalid:
    default:
    {
        setNodeState(true);
        break;
    }
    case CircuitFlags::Code75:
    case CircuitFlags::Code120:
    case CircuitFlags::Code180:
    case CircuitFlags::Code270:
    {
        const bool codeOn = modeMgr()->getCodePhase(codeFromFlag(flags));
        setNodeState(codeOn);
        break;
    }
    }
}
