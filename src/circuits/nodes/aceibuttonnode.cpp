/**
 * src/circuits/nodes/aceibuttonnode.cpp
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

#include "aceibuttonnode.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

ACEIButtonNode::ACEIButtonNode(ModeManager *mgr, QObject *parent)
    : AbstractDeviatorNode{mgr, parent}
{
    // 3 sides
    // Common
    // Pressed (Deviator Up contact)
    // Normal  (Deviator Down contact)
    setAllowSwap(false);
    setCanChangeCentralConnector(false);

    // Default state
    setContactState(mState == State::Pressed,
                    mState == State::Normal
                    || mState == State::Pressed);
}

ACEIButtonNode::~ACEIButtonNode()
{

}

bool ACEIButtonNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractDeviatorNode::loadFromJSON(obj))
        return false;

    return true;
}

void ACEIButtonNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);
}

QString ACEIButtonNode::nodeType() const
{
    return NodeType;
}

ACEIButtonNode::State ACEIButtonNode::state() const
{
    return mState;
}

void ACEIButtonNode::setState(State newState)
{
    if (mState == newState)
        return;

    mState = newState;
    setContactState(mState == State::Pressed,
                    mState == State::Normal
                    || mState == State::Pressed);
}
