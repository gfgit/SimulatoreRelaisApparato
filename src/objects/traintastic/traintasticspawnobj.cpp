/**
 * src/objects/traintastic/traintasticspawnobj.cpp
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

#include "traintasticspawnobj.h"

#include "../../circuits/nodes/traintasticturnoutnode.h"

#include "../abstractsimulationobjectmodel.h"
#include "../../views/modemanager.h"

#include "../../network/traintastic-simulator/traintasticsimmanager.h"

#include <QTimerEvent>

#include <QJsonObject>

TraintasticSpawnObj::TraintasticSpawnObj(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject(m)
{
    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->addSpawn(this);
}

TraintasticSpawnObj::~TraintasticSpawnObj()
{
    setNode(nullptr);

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->removeSpawn(this);
}

QString TraintasticSpawnObj::getType() const
{
    return Type;
}

bool TraintasticSpawnObj::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    setAddress(obj.value("address").toInt(InvalidAddress));

    return true;
}

void TraintasticSpawnObj::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["address"] = mAddress;
}

int TraintasticSpawnObj::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    if(!mNode)
        return 0;

    if(result)
        result->append(mNode);
    return 1;
}

bool TraintasticSpawnObj::setAddress(int newAddress)
{
    if(mAddress == newAddress)
        return true;

    mAddress = newAddress;
    emit settingsChanged(this);
    return true;
}

void TraintasticSpawnObj::setNode(TraintasticTurnoutNode *node)
{
    if(mNode == node)
        return;

    if(mNode)
    {
        TraintasticTurnoutNode *oldNode = mNode;
        mNode = nullptr;
        oldNode->setTurnout(nullptr);
    }

    mNode = node;
    setActive(false);

    emit settingsChanged(this);
}

void TraintasticSpawnObj::setActive(bool val)
{
    if(mActive == val)
        return;

    mActive = val;

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->setSpawnState(address(), isActive());

    emit stateChanged(this);
}
