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

#include "../../objects/traintastic/traintasticsensorobj.h"

#include "protocol.hpp"

#include <QTcpSocket>


TraintasticSimManager::TraintasticSimManager(QObject *parent)
    : QObject{parent}
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

void TraintasticSimManager::enableConnection(bool val)
{
    if(mSocket && !val)
    {
        disconnect(mSocket, nullptr, this, nullptr);
        mSocket->flush();
        mSocket->disconnectFromHost();
        mSocket->deleteLater();
        mSocket = nullptr;

        setSensorsOff();
    }

    if(!mSocket && val)
    {
        mSocket = new QTcpSocket;

        connect(mSocket, &QTcpSocket::errorOccurred,
                this, &TraintasticSimManager::onSocketError);
        connect(mSocket, &QTcpSocket::connected,
                this, &TraintasticSimManager::stateChanged);

        connect(mSocket, &QTcpSocket::disconnected,
                this, &TraintasticSimManager::onSocketError);
        connect(mSocket, &QTcpSocket::readyRead,
                this, &TraintasticSimManager::onReadyRead);

        m_readBufferOffset = 0;
        memset(m_readBuffer, 0, BufSize);

        mSocket->connectToHost(QHostAddress::LocalHost, SimulatorProtocol::DefaultPort);
        mSocket->setSocketOption(QTcpSocket::LowDelayOption, true);
    }
}

void TraintasticSimManager::onSocketError()
{
    if(!mSocket)
        return;

    disconnect(mSocket, nullptr, this, nullptr);
    mSocket->disconnectFromHost();
    mSocket->deleteLater();
    mSocket = nullptr;

    setSensorsOff();

    stateChanged();
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

void TraintasticSimManager::receive(const SimulatorProtocol::Message &message)
{
    switch (message.opCode)
    {
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

        auto chan = mTurnoutSensors.constFind(m.channel);
        if(chan == mTurnoutSensors.constEnd())
            return;

        auto it = chan->constFind(m.address);
        if(it == chan->constEnd())
            return;

        it.value()->setState(m.value);

        break;
    }
    default:
        break;
    }
}

bool TraintasticSimManager::setSensorChannel(TraintasticSensorObj *obj, int newChannel)
{
    if(obj->address() == -1)
    {
        // Still invalid
        return true;
    }

    QHash<int, QHash<int, TraintasticSensorObj *>> *map = &mSensors;

    if(obj->sensorType() == TraintasticSensorObj::SensorType::TurnoutFeedback)
    {
        map = &mTurnoutSensors;
    }

    if(newChannel != -1)
    {
        // Check if available
        auto it = map->constFind(newChannel);
        if(it != map->constEnd() && it->contains(obj->address()))
            return false; // Channel + Address combination already in use
    }

    if(obj->channel() != -1)
    {
        auto it = map->find(newChannel);
        Q_ASSERT(it != map->end());
        Q_ASSERT(it.value().value(obj->address()) == obj);

        // Remove obj from old channel
        it->remove(obj->address());

        // Remove channel if now empty
        if(it->isEmpty())
            map->erase(it);
    }

    if(newChannel != -1)
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
    if(obj->channel() == -1)
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
    Q_ASSERT(it != map->end());

    if(newAddress != -1)
    {
        // Check if available
        if(it->contains(obj->address()))
            return false; // Channel + Address combination already in use
    }

    if(obj->address() != -1)
    {
        Q_ASSERT(it.value().value(obj->address()) == obj);

        // Remove obj from old channel
        it->remove(obj->address());
    }

    if(newAddress != -1)
    {
        it->insert(obj->address(), obj);
    }

    return true;
}

void TraintasticSimManager::setSensorsOff()
{
    for(auto chan : mSensors)
    {
        for(auto sensor : chan)
        {
            sensor->setState(0);
        }
    }

    // TODO: set unknown state?
    for(auto chan : mTurnoutSensors)
    {
        for(auto sensor : chan)
        {
            sensor->setState(0);
        }
    }
}
