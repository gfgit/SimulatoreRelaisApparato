/**
 * src/serial/serialdevice.cpp
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

#include "serialdevice.h"
#include "serialmanager.h"

#include "../views/modemanager.h"

#include "../objects/circuit_bridge/remotecircuitbridge.h"
#include "../objects/circuit_bridge/remotecircuitbridgesmodel.h"

#include <QSerialPort>

SerialDevice::~SerialDevice()
{
    Q_ASSERT(!isConnected());

    const auto inputsCopy = inputs;
    for(RemoteCircuitBridge *bridge : inputsCopy)
    {
        bridge->setSerialDevice(nullptr);
    }
    Q_ASSERT(inputs.isEmpty());

    const auto outputsCopy = outputs;
    for(RemoteCircuitBridge *bridge : outputsCopy)
    {
        bridge->setSerialDevice(nullptr);
    }
    Q_ASSERT(outputs.isEmpty());
}

bool SerialDevice::setName(const QString &newName, SerialManager *mgr)
{
    const QString nameTrimmed = newName.trimmed();
    if(nameTrimmed.isEmpty() || nameTrimmed == mName)
        return false;

    if(!mgr->renameDevice(mName, nameTrimmed))
        return false;

    mName = nameTrimmed;

    ModeManager *modeMgr = mgr->modeMgr();
    modeMgr->setFileEdited();

    RemoteCircuitBridgesModel *bridgesModel = static_cast<RemoteCircuitBridgesModel *>(
                modeMgr->modelForType(RemoteCircuitBridge::Type));
    bridgesModel->updateSerialCols();

    return true;
}

void SerialDevice::onOutputChanged(int outputId, int mode)
{
    if(!isConnected())
        return;

    quint8 msg[] =
    {
        SerialManager::SerialCommands::OutputChange,
        quint8(outputId),
        quint8(mode)
    };

    SerialManager::writeMessageFlush(serialPort, msg, sizeof(msg));
}

void SerialDevice::addRemoteBridge(RemoteCircuitBridge *bridge)
{
    if(bridge->mSerialInputId)
        inputs.insert(bridge->mSerialInputId, bridge);

    if(bridge->mSerialOutputId)
        outputs.insert(bridge->mSerialOutputId, bridge);
}

void SerialDevice::removeRemoteBridge(RemoteCircuitBridge *bridge)
{
    if(bridge->mSerialInputId)
    {
        bool ok = inputs.remove(bridge->mSerialInputId);
        Q_ASSERT(ok);
    }

    if(bridge->mSerialOutputId)
    {
        bool ok = outputs.remove(bridge->mSerialOutputId);
        Q_ASSERT(ok);
    }
}

bool SerialDevice::changeRemoteBridgeInput(RemoteCircuitBridge *bridge, int oldValue, int newValue)
{
    if(oldValue == newValue)
        return true;

    if(newValue && inputs.contains(newValue))
        return false;

    if(oldValue)
    {
        auto it = inputs.constFind(oldValue);
        Q_ASSERT(it != inputs.cend() && it.value() == bridge);
        inputs.erase(it);
    }

    if(newValue)
        inputs.insert(newValue, bridge);

    return true;
}

bool SerialDevice::changeRemoteBridgeOutput(RemoteCircuitBridge *bridge, int oldValue, int newValue)
{
    if(oldValue == newValue)
        return true;

    if(newValue && outputs.contains(newValue))
        return false;

    if(oldValue)
    {
        auto it = outputs.constFind(oldValue);
        Q_ASSERT(it != outputs.cend() && it.value() == bridge);
        outputs.erase(it);
    }

    if(newValue)
        outputs.insert(newValue, bridge);

    return true;
}

void SerialDevice::reset()
{
    for(RemoteCircuitBridge *bridge : std::as_const(inputs))
    {
        bridge->mSerialNameId = 0;
        bridge->onRemoteDisconnected();
    }

    for(RemoteCircuitBridge *bridge : std::as_const(outputs))
    {
        bridge->mSerialNameId = 0;
        bridge->onRemoteDisconnected();
    }
}

void SerialDevice::closeSerial()
{
    if(!serialPort)
        return;

    serialPort->setObjectName(QString());
    serialPort->close();
    delete serialPort;
    serialPort = nullptr;
}

void SerialDevice::start(QSerialPort *serial)
{
    Q_ASSERT(serialPort == nullptr);
    serialPort = serial;

    // Reset time
    lastPingReply = QTime::currentTime();

    const quint64 deviceId = qHash(serialPort->objectName());

    for(RemoteCircuitBridge *bridge : std::as_const(inputs))
    {
        bridge->mSerialNameId = deviceId;
        bridge->onRemoteStarted();
    }

    for(RemoteCircuitBridge *bridge : std::as_const(outputs))
    {
        bridge->mSerialNameId = deviceId;
        bridge->onRemoteStarted();
    }
}

void SerialDevice::processMessage(const QByteArray &msg)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(msg.constData());

    switch (data[0])
    {
    case SerialManager::SerialCommands::NameResponse:
    {
        // Ignore
        break;
    }
    case SerialManager::SerialCommands::PingResponse:
    {
        lastPingReply = QTime::currentTime();
        break;
    }
    case SerialManager::SerialCommands::Ping:
    {
        // Send Ping response
        const quint8 c = SerialManager::SerialCommands::PingResponse;
        SerialManager::writeMessageFlush(serialPort, &c, 1);
        break;
    }
    case SerialManager::SerialCommands::InputChange:
    {
        if(msg.size() < 3)
            return;

        const quint8 inputId = data[1];
        const quint8 mode = data[2];

        RemoteCircuitBridge *bridge = inputs.value(inputId, nullptr);
        if(!bridge)
            return;

        bridge->onSerialInputMode(mode);
        break;
    }
    default:
    {
        qWarning() << "SERIAL: recv ivalid command:" << data[0] << "dev:" << serialPort->objectName();

        // TODO: reset device?
        break;
    }
    }
}
