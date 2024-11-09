/**
 * src/circuits/nodes/relaiscontactnode.cpp
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

#include "relaiscontactnode.h"

#include "../../objects/relais/model/abstractrelais.h"
#include "../../objects/relais/model/relaismodel.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

RelaisContactNode::RelaisContactNode(ModeManager *mgr, QObject *parent)
    : AbstractDeviatorNode{mgr, parent}
{

}

RelaisContactNode::~RelaisContactNode()
{
    setRelais(nullptr);
}

bool RelaisContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractDeviatorNode::loadFromJSON(obj))
        return false;

    QString relaisName = obj.value("relais").toString();
    setRelais(modeMgr()->relaisModel()->getRelay(relaisName));

    setHideRelayNormalState(obj.value("hide_relay_normal").toBool());

    return true;
}

void RelaisContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();
    obj["hide_relay_normal"] = hideRelayNormalState();
}

QString RelaisContactNode::nodeType() const
{
    return NodeType;
}

AbstractRelais *RelaisContactNode::relais() const
{
    return mRelais;
}

void RelaisContactNode::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;

    const bool hadRelay = mRelais;

    if(mRelais)
    {
        disconnect(mRelais, &AbstractRelais::stateChanged,
                   this, &RelaisContactNode::onRelaisStateChanged);

        mRelais->removeContactNode(this);
    }

    mRelais = newRelais;

    if(mRelais)
    {
        connect(mRelais, &AbstractRelais::stateChanged,
                this, &RelaisContactNode::onRelaisStateChanged);

        mRelais->addContactNode(this);

        if(!hadRelay &&
                modeMgr()->mode() == FileMode::Editing &&
                mRelais->normallyUp())
        {
            // Swap by default for new normally up ralais contacts
            setSwapContactState(true);
        }
    }

    emit relayChanged(mRelais);
    onRelaisStateChanged();
    modeMgr()->setFileEdited();
}

RelaisContactNode::State RelaisContactNode::state() const
{
    return mState;
}

void RelaisContactNode::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;

    setContactState(mState == State::Up,
                    mState == State::Down);

    emit stateChanged();
}

void RelaisContactNode::onRelaisStateChanged()
{
    State s = State::Middle;
    if(mRelais)
    {
        switch (mRelais->state())
        {
        case AbstractRelais::State::Down:
            s = State::Down;
            break;
        case AbstractRelais::State::Up:
            s = State::Up;
            break;
        default:
            break;
        }
    }

    setState(s);
}

bool RelaisContactNode::hideRelayNormalState() const
{
    return mHideRelayNormalState;
}

void RelaisContactNode::setHideRelayNormalState(bool newHideRelayNormalState)
{
    if(mHideRelayNormalState == newHideRelayNormalState)
        return;

    mHideRelayNormalState = newHideRelayNormalState;
    emit shapeChanged();
    modeMgr()->setFileEdited();
}
