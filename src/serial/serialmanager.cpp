/**
 * src/serial/serialmanager.cpp
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

#include "serialmanager.h"

#include "../objects/circuit_bridge/remotecircuitbridge.h"

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QTimerEvent>

SerialManager::SerialManager(QObject *parent)
    : QObject{parent}
{

}

SerialManager::~SerialManager()
{
    disconnectAllDevices();
}

void SerialManager::rescanPorts()
{
    bool needsRescan = false;
    for(const SerialDevice& dev : mDevices)
    {
        if(!dev.serialPort)
        {
            needsRescan = true;
            break;
        }
    }

    if(!needsRescan)
        return;

    const QList<QSerialPortInfo> allPorts = QSerialPortInfo::availablePorts();

    for(const QSerialPortInfo& info : allPorts)
    {
        bool alreadyActive = false;
        for(const auto &p : mPendingNamePorts)
        {
            if(p.second->portName() == info.portName())
            {
                alreadyActive = true;
                break;
            }
        }

        if(!alreadyActive)
        {
            for(const SerialDevice& dev : mDevices)
            {
                if(dev.serialPort && dev.serialPort->portName() == info.portName())
                {
                    alreadyActive = true;
                    break;
                }
            }
        }

        if(alreadyActive)
            continue;

        QSerialPort *serialPort = new QSerialPort(info);
        serialPort->setBaudRate(QSerialPort::Baud115200);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        if(!serialPort->open(QIODevice::ReadWrite))
        {
            qDebug() << serialPort->portName() << "ERROR:" << serialPort->error() << serialPort->errorString();
            delete serialPort;
            continue;
        }

        connect(serialPort, &QSerialPort::readyRead,
                this, &SerialManager::onSerialRead);
        connect(serialPort, &QSerialPort::errorOccurred,
                this, &SerialManager::onSerialDisconnected,
                Qt::QueuedConnection);

        mPendingNamePorts.append({QTime::currentTime(), serialPort});

        if(!mPendingClearTimer.isActive())
            mPendingClearTimer.start(5000, this);

        // Init handshake
        serialPort->write(QByteArrayLiteral("\n"
                                            "simrelais?"
                                            "\n"));
        serialPort->flush();

        if(!serialPort->waitForReadyRead(100))
            qDebug() << "ERR SERIAL";
    }
}

void SerialManager::disconnectAllDevices()
{
    for(SerialDevice& dev : mDevices)
    {
        dev.reset();

        if(dev.serialPort)
        {
            dev.serialPort->setObjectName(QString());
            dev.serialPort->close();
            delete dev.serialPort;
            dev.serialPort = nullptr;
        }
    }
}

void SerialManager::onOutputChanged(qint64 deviceId, int outputId, int mode)
{
    auto it = mDevices.find(deviceId);
    if(it == mDevices.end())
        return;

    if(!it.value().serialPort)
        return;

    QByteArray msg;
    msg.reserve(10);
    msg.append('o');
    msg.append(QByteArray::number(outputId));
    msg.append(';');
    msg.append(QByteArray::number(mode));
    msg.append('\n');

    it.value().serialPort->write(msg);
    it.value().serialPort->flush();
}

void SerialManager::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mPendingClearTimer.timerId())
    {
        const QTime now = QTime::currentTime();
        for(auto it = mPendingNamePorts.begin(); it != mPendingNamePorts.end(); )
        {
            if(it->first.msecsTo(now) > 900)
            {
                it->second->close();
                delete it->second;

                it = mPendingNamePorts.erase(it);
                continue;
            }

            it++;
        }

        if(mPendingNamePorts.isEmpty())
            mPendingClearTimer.stop();
    }
}

void SerialManager::onSerialRead()
{
    QSerialPort *serialPort = qobject_cast<QSerialPort *>(sender());

    qDebug() << "SERIAL READ:" << serialPort->objectName() << serialPort->bytesAvailable();

    if(serialPort->objectName().isEmpty())
    {
        if(!checkSerialValid(serialPort))
            return;
    }

    while(serialPort->canReadLine())
    {
        QByteArray reply = serialPort->readLine(100);
        if(reply.isEmpty())
            continue;

        if(reply.at(0) == 'i')
        {
            QByteArrayView view(reply);
            const int sep = view.indexOf(';');

            bool ok = false;
            const int inputId = view.sliced(1, sep - 1).toInt(&ok);

            if(ok)
            {
                ok = false;
                const int inputMode = view.sliced(sep + 1).toInt(&ok);

                onInputChanged(serialPort->objectName(),
                               inputId, inputMode);
            }
        }
    }
}

void SerialManager::onSerialDisconnected()
{
    QSerialPort *serialPort = qobject_cast<QSerialPort *>(sender());

    if(serialPort->objectName().isEmpty())
    {
        for(auto it2 = mPendingNamePorts.begin(); it2 != mPendingNamePorts.end(); it2++)
        {
            if(it2->second == serialPort)
            {
                mPendingNamePorts.erase(it2);
                break;
            }
        }

        if(mPendingNamePorts.isEmpty())
            mPendingClearTimer.stop();

        return;
    }

    auto it = mDevices.find(qHash(serialPort->objectName()));
    Q_ASSERT(it != mDevices.end());

    it.value().reset();

    it.value().serialPort = nullptr;
    serialPort->setObjectName(QString());
    delete serialPort;
}

bool SerialManager::checkSerialValid(QSerialPort *serialPort)
{
    for(auto it2 = mPendingNamePorts.begin(); it2 != mPendingNamePorts.end(); it2++)
    {
        if(it2->second == serialPort)
        {
            mPendingNamePorts.erase(it2);
            break;
        }
    }

    if(mPendingNamePorts.isEmpty())
        mPendingClearTimer.stop();

    if(serialPort->canReadLine())
    {
        QByteArray reply = serialPort->readLine(100);
        if(reply.startsWith("simrelais!"))
        {
            const QString name = QString::fromUtf8(reply.mid(10)).trimmed();
            if(!name.isEmpty())
            {
                auto it = mDevices.find(qHash(name));
                if(it != mDevices.end() && !it.value().serialPort)
                {
                    serialPort->setObjectName(name);
                    it.value().serialPort = serialPort;

                    it.value().start();
                    return true;
                }
            }
        }
    }

    serialPort->close();
    delete serialPort;

    return false;
}

void SerialManager::onInputChanged(const QString &name, int inputId, int mode)
{
    auto it = mDevices.find(qHash(name));
    if(it == mDevices.end())
        return;

    SerialDevice& dev = it.value();
    RemoteCircuitBridge *bridge = dev.inputs.value(inputId, nullptr);
    if(!bridge)
        return;

    bridge->onSerialInputMode(mode);
}

void SerialManager::addRemoteBridge(RemoteCircuitBridge *bridge, const QString &devName)
{
    const quint64 deviceId = qHash(devName);

    auto it = mDevices.find(deviceId);
    if(it == mDevices.end())
        it = mDevices.insert(deviceId, {});

    SerialDevice& dev = it.value();
    if(bridge->mSerialInputId)
        dev.inputs.insert(bridge->mSerialInputId, bridge);

    if(bridge->mSerialOutputId)
        dev.outputs.insert(bridge->mSerialOutputId, bridge);
}

void SerialManager::removeRemoteBridge(RemoteCircuitBridge *bridge, const QString &devName)
{
    const quint64 deviceId = qHash(devName);

    auto it = mDevices.find(deviceId);
    Q_ASSERT(it != mDevices.end());

    SerialDevice& dev = it.value();
    dev.inputs.remove(bridge->mSerialInputId);
    dev.outputs.remove(bridge->mSerialOutputId);

    if(dev.inputs.isEmpty() && dev.outputs.isEmpty())
    {
        mDevices.erase(it);
    }
}

void SerialManager::SerialDevice::reset()
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

void SerialManager::SerialDevice::start()
{
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
