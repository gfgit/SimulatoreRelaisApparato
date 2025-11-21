/**
 * src/objects/traintastic/traintasticsignalobject.cpp
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

#include "traintasticsignalobject.h"

#include "../screen_relais/model/screenrelais.h"
#include "../relais/model/abstractrelais.h"

#include "../simple_activable/lightbulbobject.h"

#include "../abstractsimulationobjectmodel.h"
#include "../../views/modemanager.h"

#include "../../network/traintastic-simulator/traintasticsimmanager.h"
#include "../../network/traintastic-simulator/protocol.hpp"

#include <QJsonObject>

TraintasticSignalObject::TraintasticSignalObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{}

TraintasticSignalObject::~TraintasticSignalObject()
{

}

QString TraintasticSignalObject::getType() const
{
    return Type;
}

bool TraintasticSignalObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase == LoadPhase::Creation)
    {
        setChannel(obj.value("channel").toInt(0));
        setAddress(obj.value("address").toInt(InvalidAddress));
        return true;
    }

    auto model_ = model()->modeMgr()->modelForType(ScreenRelais::Type);
    if(model_)
    {
        const QString fmt(QLatin1String("screen_%1"));
        for(int i = 0; i < NScreenRelays; i++)
        {
            setScreenRelaisAt(i, static_cast<ScreenRelais *>(model_->getObjectByName(
                                    obj.value(fmt.arg(i)).toString())));
        }
    }
    else
    {
        for(int i = 0; i < NScreenRelays; i++)
        {
            setScreenRelaisAt(i, nullptr);
        }
    }

    auto model2_ = model()->modeMgr()->modelForType(AbstractRelais::Type);
    if(model2_)
    {
        const QString fmt(QLatin1String("blink_%1"));
        for(int i = 0; i < NBlinkRelays; i++)
        {
            setBlinkRelaisAt(i, static_cast<AbstractRelais *>(model2_->getObjectByName(
                                    obj.value(fmt.arg(i)).toString())));
        }
    }
    else
    {
        for(int i = 0; i < NBlinkRelays; i++)
        {
            setBlinkRelaisAt(i, nullptr);
        }
    }

    auto model3_ = model()->modeMgr()->modelForType(LightBulbObject::Type);
    if(model3_)
    {
        setArrowLight(static_cast<LightBulbObject *>(model3_->getObjectByName(obj.value("arrow_light").toString())));
    }
    else
    {
        setArrowLight(nullptr);
    }

    return true;
}

void TraintasticSignalObject::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["channel"] = mChannel;
    obj["address"] = mAddress;

    const QString fmt1(QLatin1String("screen_%1"));
    for(int i = 0; i < NScreenRelays; i++)
    {
        obj[fmt1.arg(i)] = mScreenRelais[i] ? mScreenRelais[i]->name() : QString();
    }

    const QString fmt2(QLatin1String("blink_%1"));
    for(int i = 0; i < NBlinkRelays; i++)
    {
        obj[fmt2.arg(i)] = mBlinkRelais[i] ? mBlinkRelais[i]->name() : QString();
    }

    obj["arrow_light"] = mArrowLight ? mArrowLight->name() : QString();
}

void TraintasticSignalObject::setChannel(int newChannel)
{
    if(mChannel == newChannel)
        return;

    mChannel = newChannel;
    emit settingsChanged(this);
}

void TraintasticSignalObject::setAddress(int newAddress)
{
    if(mAddress == newAddress)
        return;

    mAddress = newAddress;
    emit settingsChanged(this);
}

void TraintasticSignalObject::setScreenRelaisAt(int i, ScreenRelais *s)
{
    assert(i >= 0 && i < NScreenRelays);
    if(mScreenRelais[i] == s)
        return;

    if(mScreenRelais[i])
    {
        disconnect(mScreenRelais[i], &ScreenRelais::stateChanged,
                   this, &TraintasticSignalObject::onScreenPosChanged);
        disconnect(mScreenRelais[i], &ScreenRelais::destroyed,
                   this, &TraintasticSignalObject::onScreenDestroyed);
    }

    mScreenRelais[i] = s;

    if(mScreenRelais[i])
    {
        connect(mScreenRelais[i], &ScreenRelais::stateChanged,
                this, &TraintasticSignalObject::onScreenPosChanged);
        connect(mScreenRelais[i], &ScreenRelais::destroyed,
                this, &TraintasticSignalObject::onScreenDestroyed);

        const int newPos = s->getClosestGlass();
        setScreenPos(i, newPos);
    }

    emit settingsChanged(this);
}

void TraintasticSignalObject::setBlinkRelaisAt(int i, AbstractRelais *s)
{
    assert(i >= 0 && i < NBlinkRelays);
    if(mBlinkRelais[i] == s)
        return;

    if(mBlinkRelais[i])
    {
        disconnect(mBlinkRelais[i], &AbstractRelais::stateChanged,
                   this, &TraintasticSignalObject::onBlinRelaisStateChanged);
        disconnect(mBlinkRelais[i], &AbstractRelais::destroyed,
                   this, &TraintasticSignalObject::onBlinRelaisDestroyed);
    }

    mBlinkRelais[i] = s;

    if(mBlinkRelais[i])
    {
        connect(mBlinkRelais[i], &AbstractRelais::stateChanged,
                this, &TraintasticSignalObject::onBlinRelaisStateChanged);
        connect(mBlinkRelais[i], &AbstractRelais::destroyed,
                this, &TraintasticSignalObject::onBlinRelaisDestroyed);

        sendStatusMsg();

        setBlinkRelayState(i, mBlinkRelais[i]->state() == AbstractRelais::State::Up);
    }
    else
    {
        setBlinkRelayState(i, false);
    }

    emit settingsChanged(this);
}

void TraintasticSignalObject::sendStatusMsg()
{
    SimulatorProtocol::SignalSetState msg(mChannel, mAddress);
    for(int i = 0; i < NScreenRelays; i++)
    {
        msg.lights[i].color = SimulatorProtocol::SignalSetState::Red;

        if(mScreenRelais[i] == nullptr)
        {
            msg.lights[i].state = SimulatorProtocol::SignalSetState::Off;
            continue;
        }

        // TODO: blink reverse
        msg.lights[i].state = SimulatorProtocol::SignalSetState::On;
        if(mBlinkRelaisUp[i])
            msg.lights[i].state = SimulatorProtocol::SignalSetState::Blink;

        switch (mScreenRelais[i]->getColorAt(mCurScreenPos[i]))
        {
        case ScreenRelais::GlassColor::Red:
            msg.lights[i].color = SimulatorProtocol::SignalSetState::Red;
            break;

        case ScreenRelais::GlassColor::Yellow:
            msg.lights[i].color = SimulatorProtocol::SignalSetState::Yellow;
            break;

        case ScreenRelais::GlassColor::Green:
            msg.lights[i].color = SimulatorProtocol::SignalSetState::Green;
            break;

        case ScreenRelais::GlassColor::Black:
        default:
            msg.lights[i].color = SimulatorProtocol::SignalSetState::Red;
            msg.lights[i].state = SimulatorProtocol::SignalSetState::Off;
            break;
            break;
        }
    }

    msg.setArrowLightOn(mArrowLight && mArrowLight->state() == LightBulbObject::State::On);

    auto startSignalState = SimulatorProtocol::SignalSetState::Off;
    if(mBlinkRelaisUp[StartSignalFakeOn])
    {
        if(mBlinkRelaisUp[StartSignalBlinker])
            startSignalState = SimulatorProtocol::SignalSetState::Blink;
        else
            startSignalState = SimulatorProtocol::SignalSetState::On;
    }
    msg.setStartSignalState(startSignalState);

    msg.speed = 0.0f;
    if(msg.lights[0].state != SimulatorProtocol::SignalSetState::Off)
    {
        switch (msg.lights[0].color)
        {
        case SimulatorProtocol::SignalSetState::Red:
        {
            if(msg.lights[0].state != SimulatorProtocol::SignalSetState::On)
                break; // Red cannot blink

            if(msg.lights[1].state == SimulatorProtocol::SignalSetState::Off
                || msg.lights[1].color == SimulatorProtocol::SignalSetState::Red)
                break; // Second light cannot be red

            if(msg.lights[2].state != SimulatorProtocol::SignalSetState::Off &&
                msg.lights[1].color == SimulatorProtocol::SignalSetState::Red)
                break; // Third light cannot be red

            if(msg.lights[1].color == SimulatorProtocol::SignalSetState::Green &&
                msg.lights[2].state != SimulatorProtocol::SignalSetState::Off)
                break; // If second light is Green, third must be off

            // Red + Yellow or
            // Red + Green or
            // Red + Yellow + Green

            // Deviata
            msg.speed = 30.0f; // TODO: rappel or blink of previous signal
            break;
        }

        case SimulatorProtocol::SignalSetState::Yellow:
        {
            if(msg.lights[2].state != SimulatorProtocol::SignalSetState::Off)
                break; // Third light must be off

            if(msg.lights[1].state != SimulatorProtocol::SignalSetState::Off)
            {
                if(msg.lights[1].color == SimulatorProtocol::SignalSetState::Red)
                    break; // Second light cannot be red

                // Yellow + Yellow or
                // Yellow + Green
                msg.speed = 90.0f; // TODO: distant signal
                break;
            }

            // Yellow
            msg.speed = 80.0f; // TODO: distant signal

            if(msg.lights[0].state == SimulatorProtocol::SignalSetState::Blink)
                msg.speed = 120.0f;

            break;
        }

        case SimulatorProtocol::SignalSetState::Green:
        {
            if(msg.lights[0].state != SimulatorProtocol::SignalSetState::On)
                break; // Green cannot blink

            // Green
            msg.speed = 200.0f;
            break;
        }
        default:
            break;
        }
    }

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->send(msg);
}

void TraintasticSignalObject::onScreenPosChanged(AbstractSimulationObject *s)
{
    for(int i = 0; i < NScreenRelays; i++)
    {
        if(mScreenRelais[i] != s)
            continue;

        const int newPos = mScreenRelais[i]->getClosestGlass();
        setScreenPos(i, newPos);
    }
}

void TraintasticSignalObject::onScreenDestroyed(QObject *obj)
{
    for(int i = 0; i < NScreenRelays; i++)
    {
        if(mScreenRelais[i] == obj)
        {
            disconnect(mScreenRelais[i], &ScreenRelais::destroyed,
                       this, &TraintasticSignalObject::onScreenDestroyed);
            mScreenRelais[i] = nullptr;
        }
    }
}

void TraintasticSignalObject::onBlinRelaisStateChanged(AbstractSimulationObject *s)
{
    for(int i = 0; i < NBlinkRelays; i++)
    {
        if(mBlinkRelais[i] != s)
            continue;

        setBlinkRelayState(i, mBlinkRelais[i]->state() == AbstractRelais::State::Up);
    }
}

void TraintasticSignalObject::onBlinRelaisDestroyed(QObject *obj)
{
    for(int i = 0; i < NBlinkRelays; i++)
    {
        if(mBlinkRelais[i] == obj)
        {
            disconnect(mBlinkRelais[i], &AbstractRelais::destroyed,
                       this, &TraintasticSignalObject::onBlinRelaisDestroyed);
            mBlinkRelais[i] = nullptr;
            setBlinkRelayState(i, false);
        }
    }
}

void TraintasticSignalObject::onArrowLightDestroyed(QObject *obj)
{
    if(mArrowLight == obj)
    {
        disconnect(mArrowLight, &LightBulbObject::stateChanged,
                   this, &TraintasticSignalObject::sendStatusMsg);
        disconnect(mArrowLight, &LightBulbObject::destroyed,
                   this, &TraintasticSignalObject::onArrowLightDestroyed);
        mArrowLight = nullptr;
        emit settingsChanged(this);
    }
}

void TraintasticSignalObject::setScreenPos(int idx, int glassPos)
{
    if(mCurScreenPos[idx] == glassPos)
        return;

    mCurScreenPos[idx] = glassPos;
    sendStatusMsg();
}

void TraintasticSignalObject::setBlinkRelayState(int idx, bool up)
{
    if(mBlinkRelaisUp[idx] == up)
        return;

    mBlinkRelaisUp[idx] = up;
    sendStatusMsg();
}

LightBulbObject *TraintasticSignalObject::arrowLight() const
{
    return mArrowLight;
}

void TraintasticSignalObject::setArrowLight(LightBulbObject *newArrowLight)
{
    if(mArrowLight == newArrowLight)
        return;


    if(mArrowLight)
    {
        disconnect(mArrowLight, &LightBulbObject::stateChanged,
                   this, &TraintasticSignalObject::sendStatusMsg);
        disconnect(mArrowLight, &LightBulbObject::destroyed,
                   this, &TraintasticSignalObject::onArrowLightDestroyed);
    }

    mArrowLight = newArrowLight;

    if(mArrowLight)
    {
        connect(mArrowLight, &LightBulbObject::stateChanged,
                this, &TraintasticSignalObject::sendStatusMsg);
        connect(mArrowLight, &LightBulbObject::destroyed,
                this, &TraintasticSignalObject::onArrowLightDestroyed);
    }

    sendStatusMsg();

    emit settingsChanged(this);
}
