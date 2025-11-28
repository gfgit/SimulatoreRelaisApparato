/**
 * src/objects/traintastic/traintasticauxsignalobject.cpp
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

#include "traintasticauxsignalobject.h"

#include "../simple_activable/lightbulbobject.h"

#include "../abstractsimulationobjectmodel.h"
#include "../../views/modemanager.h"

#include "../../network/traintastic-simulator/traintasticsimmanager.h"
#include "../../network/traintastic-simulator/protocol.hpp"

#include <QJsonObject>

TraintasticAuxSignalObject::TraintasticAuxSignalObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{}

TraintasticAuxSignalObject::~TraintasticAuxSignalObject()
{
    for(int i = 0; i < int(AuxLights::NAuxLights); i++)
    {
        setAuxLight(nullptr, AuxLights(i));
    }
}

QString TraintasticAuxSignalObject::getType() const
{
    return Type;
}

bool TraintasticAuxSignalObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase == LoadPhase::Creation)
    {
        setChannel(obj.value("channel").toInt(0));
        setAddress(obj.value("address").toInt(InvalidAddress));
        return true;
    }

    auto lightsModel = model()->modeMgr()->modelForType(LightBulbObject::Type);
    if(lightsModel)
    {
        setAuxLight(static_cast<LightBulbObject *>(lightsModel->getObjectByName(obj.value("dwarf_L1").toString())),
                    AuxLights::L1);
        setAuxLight(static_cast<LightBulbObject *>(lightsModel->getObjectByName(obj.value("dwarf_L2").toString())),
                    AuxLights::L2);
        setAuxLight(static_cast<LightBulbObject *>(lightsModel->getObjectByName(obj.value("dwarf_L3").toString())),
                    AuxLights::L3);
    }
    else
    {
        setAuxLight(nullptr, AuxLights::L1);
        setAuxLight(nullptr, AuxLights::L2);
        setAuxLight(nullptr, AuxLights::L3);
    }

    return true;
}

void TraintasticAuxSignalObject::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["channel"] = mChannel;
    obj["address"] = mAddress;

    obj["dwarf_L1"] = auxLight(AuxLights::L1) ? auxLight(AuxLights::L1)->name() : QString();
    obj["dwarf_L2"] = auxLight(AuxLights::L2) ? auxLight(AuxLights::L2)->name() : QString();
    obj["dwarf_L3"] = auxLight(AuxLights::L3) ? auxLight(AuxLights::L3)->name() : QString();
}

void TraintasticAuxSignalObject::setChannel(int newChannel)
{
    if(mChannel == newChannel)
        return;

    mChannel = newChannel;
    emit settingsChanged(this);
}

void TraintasticAuxSignalObject::setAddress(int newAddress)
{
    if(mAddress == newAddress)
        return;

    mAddress = newAddress;
    emit settingsChanged(this);
}

void TraintasticAuxSignalObject::sendStatusMsg()
{
    SimulatorProtocol::AuxSignalSetState msg(mChannel, mAddress);
    for(uint8_t i = 0; i < int(AuxLights::NAuxLights); i++)
    {
        LightBulbObject *light = auxLight(AuxLights(i));
        msg.setLightOn(i, light && light->state() == LightBulbObject::State::On);
    }

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->send(msg);
}

void TraintasticAuxSignalObject::onAuxLightDestroyed(QObject *obj)
{
    for(LightBulbObject *&light : mLights)
    {
        if(light != obj)
            continue;

        disconnect(light, &LightBulbObject::stateChanged,
                   this, &TraintasticAuxSignalObject::sendStatusMsg);
        disconnect(light, &LightBulbObject::destroyed,
                   this, &TraintasticAuxSignalObject::onAuxLightDestroyed);
        light = nullptr;
    }

    emit settingsChanged(this);
}

LightBulbObject *TraintasticAuxSignalObject::auxLight(AuxLights l) const
{
    switch (l)
    {
    case AuxLights::L1:
    case AuxLights::L2:
    case AuxLights::L3:
        break;
    default:
        return nullptr;
    }

    return mLights[int(l)];
}

void TraintasticAuxSignalObject::setAuxLight(LightBulbObject *newArrowLight, AuxLights l)
{
    switch (l)
    {
    case AuxLights::L1:
    case AuxLights::L2:
    case AuxLights::L3:
        break;
    default:
        return;
    }

    LightBulbObject *&light = mLights[int(l)];
    if(light == newArrowLight)
        return;

    if(light)
    {
        disconnect(light, &LightBulbObject::stateChanged,
                   this, &TraintasticAuxSignalObject::sendStatusMsg);
        disconnect(light, &LightBulbObject::destroyed,
                   this, &TraintasticAuxSignalObject::onAuxLightDestroyed);
    }

    light = newArrowLight;

    if(light)
    {
        connect(light, &LightBulbObject::stateChanged,
                this, &TraintasticAuxSignalObject::sendStatusMsg);
        connect(light, &LightBulbObject::destroyed,
                this, &TraintasticAuxSignalObject::onAuxLightDestroyed);
    }

    sendStatusMsg();

    emit settingsChanged(this);
}
