/**
 * src/network/traintastic-simulator/traintasticsimmanager.cpp
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

#include "traintasticsimmanager.h"

#include "../../views/modemanager.h"
#include "../remotemanager.h"
#include "../peermanager.h"

#include "../../objects/traintastic/traintasticsensorobj.h"
#include "../../objects/traintastic/traintasticturnoutobj.h"
#include "../../objects/traintastic/traintasticsignalobject.h"
#include "../../objects/traintastic/traintasticauxsignalobject.h"
#include "../../objects/traintastic/traintasticspawnobj.h"

#include "../../objects/abstractsimulationobjectmodel.h"

#include "protocol.hpp"

#include <QTcpSocket>
#include <QTimerEvent>
#include <QTime>

TraintasticSimManager::TraintasticSimManager(ModeManager *mgr)
    : QObject{mgr}
    , mModeMgr(mgr)
{

}

TraintasticSimManager::~TraintasticSimManager()
{
    enableConnection(false);
}

bool TraintasticSimManager::isConnected() const
{
    return mSocket && mSocket->state() == QTcpSocket::ConnectedState;
}

void TraintasticSimManager::tryConnectToServer(const QHostAddress& addr, quint16 port)
{
    qDebug() << "Traintastic: trying to connect to:" << addr << port;

    if(mSocket)
    {
        qDebug() << "Traintastic: already connecting... skip";
        return; // Already connected or trying to connect
    }

    mSocket = new QTcpSocket;

    connect(mSocket, &QTcpSocket::errorOccurred,
            this, &TraintasticSimManager::onSocketError);
    connect(mSocket, &QTcpSocket::connected,
            this, &TraintasticSimManager::onConnected);

    connect(mSocket, &QTcpSocket::disconnected,
            this, &TraintasticSimManager::onSocketError);
    connect(mSocket, &QTcpSocket::readyRead,
            this, &TraintasticSimManager::onReadyRead);

    m_readBufferOffset = 0;
    memset(m_readBuffer, 0, BufSize);

    mSocket->connectToHost(addr, port);
    mSocket->setSocketOption(QTcpSocket::LowDelayOption, true);
}

void TraintasticSimManager::enableConnection(bool val)
{
    if(mSocket && !val)
    {
        disconnectSimulator();
    }
    // else if(!mSocket && val)
    //     tryConnectToServer(QHostAddress::LocalHost, 5741);
}

void TraintasticSimManager::setTurnoutState(int channel, int address, int state)
{
    SimulatorProtocol::AccessorySetState message(channel, address, state);
    send(message);
}

void TraintasticSimManager::setSpawnState(int address, bool active)
{
    SimulatorProtocol::SpawnStateChange message(address,
                                                active ?
                                                    SimulatorProtocol::SpawnStateChange::State::RequestActivate :
                                                    SimulatorProtocol::SpawnStateChange::State::Reset);
    send(message);
}

void TraintasticSimManager::onSocketError()
{
    qDebug() << "Traintastic TCP error:" << mSocket->error() << mSocket->errorString();

    disconnectSimulator();

    // Try to reconnect
    mModeMgr->getRemoteManager()->setTraintasticDiscoveryEnabled(true);
}

void TraintasticSimManager::onReadyRead()
{
    while(mSocket->bytesAvailable() != 0)
    {
        qint64 bytesTransferred = mSocket->read(reinterpret_cast<char *>(m_readBuffer + m_readBufferOffset),
                                                BufSize - m_readBufferOffset);

        const std::byte* pos = m_readBuffer;
        bytesTransferred += m_readBufferOffset;

        while(bytesTransferred > 1)
        {
            const auto* message = reinterpret_cast<const SimulatorProtocol::Message*>(pos);

            if(bytesTransferred < message->size)
            {
                break;
            }

            receive(*message);
            pos += message->size;
            bytesTransferred -= message->size;
        }

        if(bytesTransferred != 0)
        {
            memmove(m_readBuffer, pos, bytesTransferred);
        }
        m_readBufferOffset = bytesTransferred;
    }
}

void TraintasticSimManager::onConnected()
{
    emit stateChanged();

    send(SimulatorProtocol::HandShake(true));

    // Send initial turnout status
    for(const TraintasticTurnoutObj *obj : std::as_const(mTurnouts))
    {
        setTurnoutState(obj->channel(), obj->address(), int(obj->state()));
    }

    send(SimulatorProtocol::HandShake(true));

    // Send initial main signal status
    auto auxSignalsModel = mModeMgr->modelForType(TraintasticAuxSignalObject::Type);
    for(int i = 0; i < auxSignalsModel->rowCount(); i++)
    {
        TraintasticAuxSignalObject *signalObj = static_cast<TraintasticAuxSignalObject *>(auxSignalsModel->objectAt(i));

        // Tell simulator we own this dwarf signal
        SimulatorProtocol::OwnSignal msg(signalObj->channel(), signalObj->address(), 0);
        send(msg);

        // Send initial status
        signalObj->sendStatusMsg();
    }

    send(SimulatorProtocol::HandShake(true));

    mSocket->flush();

    // Send initial dwarf signal status
    auto signalsModel = mModeMgr->modelForType(TraintasticSignalObject::Type);
    for(int i = 0; i < signalsModel->rowCount(); i++)
    {
        TraintasticSignalObject *signalObj = static_cast<TraintasticSignalObject *>(signalsModel->objectAt(i));

        // Tell simulator we own this main signal
        SimulatorProtocol::OwnSignal msg(signalObj->channel(), signalObj->address(), 1);
        send(msg);

        // Send initial status
        signalObj->sendStatusMsg();
    }

    send(SimulatorProtocol::HandShake(true));

    mSocket->flush();


    // Send initial spawn status
    for(const TraintasticSpawnObj *obj : std::as_const(mSpawns))
    {
        // Tell simulator we own this spawn
        SimulatorProtocol::OwnSpawn msg(obj->address());
        send(msg);

        // Send initial status
        setSpawnState(obj->address(), obj->isActive());
    }

    send(SimulatorProtocol::HandShake(true));

    mSocket->flush();

    // Force sync track circuits by requesting their state
    for(auto it : mSensors.asKeyValueRange())
    {
        // Tell simulator we are interested in this sensor channel
        SimulatorProtocol::RequestChannel msg(it.first);
        send(msg);
    }

    mModeMgr->getRemoteManager()->setTraintasticDiscoveryEnabled(false);

    send(SimulatorProtocol::HandShake(true));
    mHandShakeTimer.start(HandShakeRate, Qt::PreciseTimer, this);
}

void TraintasticSimManager::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == mHandShakeTimer.timerId())
    {
        qDebug() << "TIMEOUT HANDSHAKE" << QTime::currentTime();
        // We missed handshake, maybe server is dead?
        disconnectSimulator();

        // Try to reconnect
        mModeMgr->getRemoteManager()->setTraintasticDiscoveryEnabled(true);
        return;
    }

    QObject::timerEvent(ev);
}

void TraintasticSimManager::send(const SimulatorProtocol::Message &message)
{
    if(!isConnected())
        return;

    mSocket->write(reinterpret_cast<const char *>(&message), message.size);
    //mSocket->flush();
}

void TraintasticSimManager::receive(const SimulatorProtocol::Message &message)
{
    switch (message.opCode)
    {
    case SimulatorProtocol::OpCode::Handshake:
    {
        // Restart counting
        mHandShakeTimer.start(HandShakeRate, Qt::PreciseTimer, this);
        send(SimulatorProtocol::HandShake(true));
        break;
    }
    case SimulatorProtocol::OpCode::AccessorySetState:
    {
        const auto& m = static_cast<const SimulatorProtocol::AccessorySetState&>(message);

        auto chan = mTurnoutSensors.constFind(m.channel);
        if(chan == mTurnoutSensors.constEnd())
            return;

        auto it = chan->constFind(m.address);
        if(it == chan->constEnd())
            return;

        it.value()->setState(m.state);

        break;
    }
    case SimulatorProtocol::OpCode::SensorChanged:
    {
        const auto& m = static_cast<const SimulatorProtocol::SensorChanged&>(message);

        auto chan = mSensors.constFind(m.channel);
        if(chan == mSensors.constEnd())
            return;

        auto it = chan->constFind(m.address);
        if(it == chan->constEnd())
            return;

        it.value()->setState(m.value);

        break;
    }
    case SimulatorProtocol::OpCode::SpawnStateChange:
    {
        const auto& m = static_cast<const SimulatorProtocol::SpawnStateChange&>(message);

        auto it = mSpawnSensors.constFind(m.address);
        if(it == mSpawnSensors.constEnd())
            return;

        it.value()->setState(m.state);

        break;
    }
    default:
        break;
    }
}

bool TraintasticSimManager::setSensorChannel(TraintasticSensorObj *obj, int newChannel)
{
    if(obj->address() == TraintasticSensorObj::InvalidAddress || obj->sensorType() == TraintasticSensorObj::SensorType::Spawn)
    {
        // Still invalid
        return true;
    }

    QHash<int, QHash<int, TraintasticSensorObj *>> *map = &mSensors;

    if(obj->sensorType() == TraintasticSensorObj::SensorType::TurnoutFeedback)
    {
        map = &mTurnoutSensors;
    }

    if(newChannel != TraintasticSensorObj::InvalidChannel)
    {
        // Check if available
        auto it = map->constFind(newChannel);
        if(it != map->constEnd() && it->contains(obj->address()))
            return false; // Channel + Address combination already in use
    }

    if(obj->channel() != TraintasticSensorObj::InvalidChannel)
    {
        auto it = map->find(obj->channel());
        Q_ASSERT(it != map->end());
        Q_ASSERT(it.value().value(obj->address()) == obj);

        // Remove obj from old channel
        it->remove(obj->address());

        // Remove channel if now empty
        if(it->isEmpty())
            map->erase(it);
    }

    if(newChannel != TraintasticSensorObj::InvalidChannel)
    {
        auto it = map->find(newChannel);
        if(it == map->end())
            it = map->insert(newChannel, {});

        it->insert(obj->address(), obj);
    }

    return true;
}

bool TraintasticSimManager::setSensorAddress(TraintasticSensorObj *obj, int newAddress)
{
    if(obj->sensorType() == TraintasticSensorObj::SensorType::Spawn)
    {
        if(newAddress != TraintasticSensorObj::InvalidAddress)
        {
            if(mSpawnSensors.contains(newAddress))
                return false; // Address already taken
        }

        if(obj->address() != TraintasticSensorObj::InvalidAddress)
            mSpawnSensors.remove(obj->address());

        if(newAddress != TraintasticSensorObj::InvalidAddress)
            mSpawnSensors.insert(newAddress, obj);

        return true;
    }

    if(obj->channel() == TraintasticSensorObj::InvalidChannel)
    {
        // Still invalid
        return true;
    }

    QHash<int, QHash<int, TraintasticSensorObj *>> *map = &mSensors;

    if(obj->sensorType() == TraintasticSensorObj::SensorType::TurnoutFeedback)
    {
        map = &mTurnoutSensors;
    }

    auto it = map->find(obj->channel());
    Q_ASSERT(obj->address() == TraintasticSensorObj::InvalidAddress || it != map->end());

    if(newAddress != TraintasticSensorObj::InvalidAddress)
    {
        // Check if available
        if(it != map->end() && it->contains(newAddress))
            return false; // Channel + Address combination already in use
    }

    if(obj->address() != TraintasticSensorObj::InvalidAddress)
    {
        Q_ASSERT(it.value().value(obj->address()) == obj);

        // Remove obj from old channel
        it->remove(obj->address());
    }

    if(newAddress != TraintasticSensorObj::InvalidAddress)
    {
        if(it == map->end())
            it = map->insert(obj->channel(), {});

        it->insert(newAddress, obj);
    }

    return true;
}

void TraintasticSimManager::addTurnout(TraintasticTurnoutObj *obj)
{
    Q_ASSERT(!mTurnouts.contains(obj));
    mTurnouts.append(obj);
}

void TraintasticSimManager::removeTurnout(TraintasticTurnoutObj *obj)
{
    Q_ASSERT(mTurnouts.contains(obj));
    mTurnouts.removeOne(obj);
}

void TraintasticSimManager::addSpawn(TraintasticSpawnObj *obj)
{
    Q_ASSERT(!mSpawns.contains(obj));
    mSpawns.append(obj);
}

void TraintasticSimManager::removeSpawn(TraintasticSpawnObj *obj)
{
    Q_ASSERT(mSpawns.contains(obj));
    mSpawns.removeOne(obj);
}

void TraintasticSimManager::setSensorsOff()
{
    for(auto chan : mSensors)
    {
        for(auto sensor : chan)
        {
            sensor->onTraintasticDisconnected();
        }
    }

    for(auto chan : mTurnoutSensors)
    {
        for(auto sensor : chan)
        {
            sensor->onTraintasticDisconnected();
        }
    }

    for(auto sensor : mSpawnSensors)
    {
        sensor->onTraintasticDisconnected();
    }
}

void TraintasticSimManager::disconnectSimulator()
{
    if(!mSocket)
        return;

    mHandShakeTimer.stop();
    disconnect(mSocket, nullptr, this, nullptr);
    mSocket->disconnectFromHost();
    mSocket->deleteLater();
    mSocket = nullptr;

    setSensorsOff();
    emit stateChanged();
}
