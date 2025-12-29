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
#include <QJsonArray>

TraintasticSignalObject::TraintasticSignalObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{}

TraintasticSignalObject::~TraintasticSignalObject()
{
    for(int i = 0; i < NScreenRelays; i++)
    {
        setScreenRelaisAt(i, nullptr);
        mCurScreenPos[i] = 0.0f;
    }

    for(int i = 0; i < NBlinkRelays; i++)
    {
        setBlinkRelaisAt(i, nullptr);
        mBlinkRelaisUp[i] = false;
    }

    setAuxLight(nullptr, AuxLights::ArrowLight);
    setAuxLight(nullptr, AuxLights::RappelLight60);
    setAuxLight(nullptr, AuxLights::RappelLight100);

    setDirectionLights({});
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

    auto lightsModel = model()->modeMgr()->modelForType(LightBulbObject::Type);
    if(lightsModel)
    {
        setAuxLight(static_cast<LightBulbObject *>(lightsModel->getObjectByName(obj.value("arrow_light").toString())),
                    AuxLights::ArrowLight);
        setAuxLight(static_cast<LightBulbObject *>(lightsModel->getObjectByName(obj.value("rappel_60").toString())),
                    AuxLights::RappelLight60);
        setAuxLight(static_cast<LightBulbObject *>(lightsModel->getObjectByName(obj.value("rappel_100").toString())),
                    AuxLights::RappelLight100);
    }
    else
    {
        setAuxLight(nullptr, AuxLights::ArrowLight);
        setAuxLight(nullptr, AuxLights::RappelLight60);
        setAuxLight(nullptr, AuxLights::RappelLight100);
    }

    const QJsonArray directionsArr = obj.value("directions").toArray();
    if(lightsModel && !directionsArr.isEmpty())
    {
        QVector<DirectionEntry> newEntries;
        newEntries.reserve(directionsArr.size());

        for(const QJsonValue& v : directionsArr)
        {
            QJsonObject entryObj = v.toObject();
            DirectionEntry entry;
            entry.light = static_cast<LightBulbObject *>(lightsModel->getObjectByName(entryObj.value("light").toString()));
            const QString str = entryObj.value("letter").toString();

            if(!entry.light || str.isEmpty())
                continue;
            entry.letter = str.at(0).toLatin1();
            newEntries.append(entry);
        }

        setDirectionLights(newEntries);
    }
    else
    {
        setDirectionLights({});
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
    obj["rappel_60"] = mRappelLight60 ? mRappelLight60->name() : QString();
    obj["rappel_100"] = mRappelLight100 ? mRappelLight100->name() : QString();

    QJsonArray directionsArr;
    for(const DirectionEntry& entry : mDirectionLights)
    {
        if(!entry.light)
            continue;

        QJsonObject entryObj;
        entryObj["light"] = entry.light->name();
        entryObj["letter"] = QString::fromLatin1(&entry.letter, 1);
        directionsArr.append(entryObj);
    }
    obj["directions"] = directionsArr;
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

    auto advanceSignalState = SimulatorProtocol::SignalSetState::Off;
    if(mBlinkRelaisUp[AdvanceSignalFakeOn])
    {
        if(mBlinkRelaisUp[AdvanceSignalBlinker])
            advanceSignalState = SimulatorProtocol::SignalSetState::Blink;
        else
            advanceSignalState = SimulatorProtocol::SignalSetState::On;
    }
    msg.setAdvanceSignalState(advanceSignalState);

    if(mRappelLight100 && mRappelLight100->state() == LightBulbObject::State::On)
        msg.rappelState = SimulatorProtocol::SignalSetState::TwoLines_100;
    else if(mRappelLight60 && mRappelLight60->state() == LightBulbObject::State::On)
        msg.rappelState = SimulatorProtocol::SignalSetState::OneLine_60;
    else
        msg.rappelState = SimulatorProtocol::SignalSetState::Rappel_Off;

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

            // Deviata, use Rappel to distinguish
            // TODO: use prev signal when no rappel
            msg.speed = 30.0f;
            if(mRappelLight100 && mRappelLight100->state() == LightBulbObject::State::On)
                msg.speed = 100.0f;
            else if(mRappelLight60 && mRappelLight60->state() == LightBulbObject::State::On)
                msg.speed = 60.0f;
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

    // Direction indicator
    msg.directionIndication = ' '; // Off
    for(const DirectionEntry& entry : mDirectionLights)
    {
        if(entry.light->state() == LightBulbObject::State::On)
        {
            msg.directionIndication = entry.letter;
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

void TraintasticSignalObject::onAuxLightDestroyed(QObject *obj)
{
    for(int i = 0; i < AuxLights::NAuxLights; i++)
    {
        LightBulbObject *light = auxLight(AuxLights(i));
        if(light != obj)
            continue;

        disconnect(light, &LightBulbObject::stateChanged,
                   this, &TraintasticSignalObject::sendStatusMsg);
        disconnect(light, &LightBulbObject::destroyed,
                   this, &TraintasticSignalObject::onAuxLightDestroyed);
        setAuxLight(nullptr, AuxLights(i));
    }

    for(int i = 0; i < mDirectionLights.size(); i++)
    {
        if(mDirectionLights.at(i).light != obj)
            continue;

        disconnect(mDirectionLights.at(i).light, &LightBulbObject::stateChanged,
                   this, &TraintasticSignalObject::sendStatusMsg);
        disconnect(mDirectionLights.at(i).light, &LightBulbObject::destroyed,
                   this, &TraintasticSignalObject::onAuxLightDestroyed);

        mDirectionLights.removeAt(i);
    }

    emit settingsChanged(this);
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

QVector<TraintasticSignalObject::DirectionEntry> TraintasticSignalObject::directionLights() const
{
    return mDirectionLights;
}

void TraintasticSignalObject::setDirectionLights(const QVector<DirectionEntry> &newDirectionLights)
{
    if(mDirectionLights == newDirectionLights)
        return;

    for(int i = 0; i < mDirectionLights.size(); i++)
    {
        disconnect(mDirectionLights.at(i).light, &LightBulbObject::stateChanged,
                   this, &TraintasticSignalObject::sendStatusMsg);
        disconnect(mDirectionLights.at(i).light, &LightBulbObject::destroyed,
                   this, &TraintasticSignalObject::onAuxLightDestroyed);
    }

    mDirectionLights.clear();
    mDirectionLights.reserve(newDirectionLights.size());
    for(const DirectionEntry& entry : newDirectionLights)
    {
        if(!entry.light)
            continue;

        QChar ch = QChar::fromLatin1(entry.letter);
        if(!ch.isLetterOrNumber())
            continue;

        bool duplicate = false;
        for(const DirectionEntry& other : mDirectionLights)
        {
            if(other.letter == entry.letter || other.light == entry.light)
            {
                duplicate = true;
                break;
            }
        }

        if(duplicate)
            continue;

        connect(entry.light, &LightBulbObject::stateChanged,
                this, &TraintasticSignalObject::sendStatusMsg);
        connect(entry.light, &LightBulbObject::destroyed,
                this, &TraintasticSignalObject::onAuxLightDestroyed);

        mDirectionLights.append(entry);
    }

    emit settingsChanged(this);
}

LightBulbObject *TraintasticSignalObject::auxLight(AuxLights l) const
{
    switch (l)
    {
    case AuxLights::ArrowLight:
        return mArrowLight;
    case AuxLights::RappelLight60:
        return mRappelLight60;
    case AuxLights::RappelLight100:
        return mRappelLight100;
    default:
        break;
    }

    return nullptr;
}

void TraintasticSignalObject::setAuxLight(LightBulbObject *newArrowLight, AuxLights l)
{
    LightBulbObject **light = nullptr;
    switch (l)
    {
    case AuxLights::ArrowLight:
        light = &mArrowLight;
        break;
    case AuxLights::RappelLight60:
        light = &mRappelLight60;
        break;
    case AuxLights::RappelLight100:
        light = &mRappelLight100;
        break;
    default:
        return;
    }

    if(*light == newArrowLight)
        return;

    if(*light)
    {
        disconnect(*light, &LightBulbObject::stateChanged,
                   this, &TraintasticSignalObject::sendStatusMsg);
        disconnect(*light, &LightBulbObject::destroyed,
                   this, &TraintasticSignalObject::onAuxLightDestroyed);
    }

    *light = newArrowLight;

    if(*light)
    {
        connect(*light, &LightBulbObject::stateChanged,
                this, &TraintasticSignalObject::sendStatusMsg);
        connect(*light, &LightBulbObject::destroyed,
                this, &TraintasticSignalObject::onAuxLightDestroyed);
    }

    sendStatusMsg();

    emit settingsChanged(this);
}
