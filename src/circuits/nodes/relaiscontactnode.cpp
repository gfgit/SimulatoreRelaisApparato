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
#include "../../objects/abstractsimulationobjectmodel.h"

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

    auto model = modeMgr()->modelForType(AbstractRelais::Type);
    if(model)
    {
        const QString relaisName = obj.value("relais").toString();
        AbstractSimulationObject *relayObj = model->getObjectByName(relaisName);

        // Do not auto swap based on relay type.
        // We do it only for newly created items during editing
        setRelais(static_cast<AbstractRelais *>(relayObj), false);
    }
    else
        setRelais(nullptr);

    setHideRelayNormalState(obj.value("hide_relay_normal").toBool());
    setActiveWhileMiddle(obj.value("active_during_middle").toBool());

    return true;
}

void RelaisContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();
    obj["hide_relay_normal"] = hideRelayNormalState();
    obj["active_during_middle"] = activeWhileMiddle();
}

QString RelaisContactNode::nodeType() const
{
    return NodeType;
}

AbstractRelais *RelaisContactNode::relais() const
{
    return mRelais;
}

void RelaisContactNode::setRelais(AbstractRelais *newRelais, bool autoSwapState)
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

    setActiveWhileMiddle(activeWhileMiddle());

    if(mRelais)
    {
        connect(mRelais, &AbstractRelais::stateChanged,
                this, &RelaisContactNode::onRelaisStateChanged);

        mRelais->addContactNode(this);

        if(autoSwapState && !hadRelay &&
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

    const bool canMiddle = (mState == State::Middle && activeWhileMiddle());

    setContactState(mState == State::Up || (canMiddle && swapContactState()),
                    mState == State::Down || (canMiddle && !swapContactState()));
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

bool RelaisContactNode::activeWhileMiddle() const
{
    if(!mActiveWhileMiddle)
        return false;

    // Can be special contact?
    if(!mRelais || hasCentralConnector())
        return false;

    if(mRelais->relaisType() != AbstractRelais::RelaisType::Combinator)
        return false;

    return true;
}

void RelaisContactNode::setActiveWhileMiddle(bool newActiveWhileMiddle)
{
    if(mActiveWhileMiddle == newActiveWhileMiddle)
        return;

    mActiveWhileMiddle = newActiveWhileMiddle;
    onRelaisStateChanged();

    emit shapeChanged();
    modeMgr()->setFileEdited();
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
