/**
 * src/circuits/nodes/screenrelaispowernode.cpp
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

#include "screenrelaispowernode.h"

#include "../../objects/screen_relais/model/screenrelais.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

ScreenRelaisPowerNode::ScreenRelaisPowerNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));
}

ScreenRelaisPowerNode::~ScreenRelaisPowerNode()
{
    setScreenRelais(nullptr);
}

QVector<CableItem> ScreenRelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0)
        return {};

    if(!screenRelais())
        return{};

    // Close the circuit
    CableItem dest;
    dest.cable.cable = mContacts.at(source.nodeContact).cable;
    dest.cable.side = mContacts.at(source.nodeContact).cableSide;
    dest.nodeContact = source.nodeContact;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void ScreenRelaisPowerNode::addCircuit(ElectricCircuit *circuit)
{
    AbstractCircuitNode::addCircuit(circuit);
    updateScreenPower();
}

void ScreenRelaisPowerNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    AbstractCircuitNode::removeCircuit(circuit, items);
    updateScreenPower();
}

bool ScreenRelaisPowerNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    auto model = modeMgr()->modelForType(ScreenRelais::Type);
    if(model)
    {
        const QString relaisName = obj.value("screen_relais").toString();
        AbstractSimulationObject *relayObj = model->getObjectByName(relaisName);
        setScreenRelais(static_cast<ScreenRelais *>(relayObj));
    }
    else
        setScreenRelais(nullptr);

    return true;
}

void ScreenRelaisPowerNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["screen_relais"] = mScreenRelais ? mScreenRelais->name() : QString();
}

QString ScreenRelaisPowerNode::nodeType() const
{
    return NodeType;
}

ScreenRelais *ScreenRelaisPowerNode::screenRelais() const
{
    return mScreenRelais;
}

bool ScreenRelaisPowerNode::setScreenRelais(ScreenRelais *newRelais, bool force)
{
    if(mScreenRelais == newRelais)
        return true;

    if(!force && newRelais && newRelais->hasPowerNode())
        return false; // New screen relay is already powered

    if(mScreenRelais)
    {
        ScreenRelais *oldScreenRelais = mScreenRelais;

        oldScreenRelais->setPowerNode(nullptr);

        disconnect(oldScreenRelais, &ScreenRelais::typeChanged,
                   this, &ScreenRelaisPowerNode::onScreenTypeChanged);
    }

    mScreenRelais = newRelais;

    if(mScreenRelais)
    {
        mScreenRelais->setPowerNode(this);

        connect(mScreenRelais, &ScreenRelais::typeChanged,
                this, &ScreenRelaisPowerNode::onScreenTypeChanged);

        updateScreenPower();
    }

    // Check new relay type
    onScreenTypeChanged();

    emit relayChanged(mScreenRelais);
    modeMgr()->setFileEdited();

    return true;
}

void ScreenRelaisPowerNode::onScreenTypeChanged()
{
    // TODO: ventola decentrata
}

void ScreenRelaisPowerNode::updateScreenPower()
{
    if(!mScreenRelais)
        return;

    ScreenRelais::PowerState s = ScreenRelais::PowerState::None;

    if(hasEntranceCircuitOnPole(0, CircuitPole::First, CircuitType::Closed))
    {
        s = ScreenRelais::PowerState::Direct;
    }
    else if(hasEntranceCircuitOnPole(0, CircuitPole::Second, CircuitType::Closed))
    {
        s = ScreenRelais::PowerState::Reversed;
    }

    mScreenRelais->setPowerState(s);
}
