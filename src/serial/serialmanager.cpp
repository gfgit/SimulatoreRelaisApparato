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
#include "serialdevice.h"
#include "serialdevicesmodel.h"

#include "../objects/circuit_bridge/remotecircuitbridge.h"

#include "../views/modemanager.h"

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QTimerEvent>

constexpr quint8 StartByte  = 0x02;
constexpr quint8 EndByte    = 0x03;
constexpr quint8 EscapeByte = 0x10;

QByteArray readMessage(QIODevice *dev, bool &reachedEnd)
{
    reachedEnd = false;

    quint8 buffer[256];
    qint64 sz = dev->peek(reinterpret_cast<char *>(buffer),
                          sizeof(buffer));

    int startPos = -1;
    for(int i = 0; i < sz; i++)
    {
        if(buffer[i] == StartByte)
        {
            startPos = i;
            break;
        }
    }

    if(startPos < 0)
    {
        dev->skip(sz);
        return {};
    }

    // Skip garbage before message start
    dev->skip(startPos);

    QByteArray message;
    message.reserve(sz - startPos - 1);

    bool escape = false;
    for(int i = startPos + 1; i < sz; i++)
    {
        quint8 c = buffer[i];
        if(c == EscapeByte)
        {
            escape = true;
            continue;
        }
        else if(c == EndByte && !escape)
        {
            // Message is complete
            dev->skip(i - startPos + 1);
            return message;
        }
        else if(c == StartByte || c == EndByte)
        {
            // Invalid message
            dev->skip(i - startPos);

            // Try again
            return {};
        }

        if(escape)
        {
            c = c ^ EscapeByte;
            escape = false;
        }

        message.append(c);
    }

    // We reached buffer end, wait more data
    reachedEnd = true;
    return {};
}

void writeMessage(QIODevice *dev, const quint8 *data, int size)
{
    quint8 buf[128] = {};
    buf[0] = StartByte;

    size_t bufPos = 1;

    for(int dataPos = 0; dataPos < size; dataPos++)
    {
        if(bufPos >= sizeof(buf) - 2)
        {
            dev->write(reinterpret_cast<const char *>(buf), bufPos);
            bufPos = 0;
        }

        quint8 c = data[dataPos];
        if(c == StartByte ||
                c == EndByte ||
                c == EscapeByte)
        {
            buf[bufPos++] = EscapeByte;
            c = c ^ EscapeByte;
        }

        buf[bufPos++] = c;
    }

    buf[bufPos++] = EndByte;
    dev->write(reinterpret_cast<const char *>(buf), bufPos);
}

void SerialManager::writeMessageFlush(QSerialPort *dev, const quint8 *data, int size)
{
    ::writeMessage(dev, data, size);
    dev->flush();
}

bool SerialManager::renameDevice(const QString &fromName, const QString &toName)
{
    if(fromName == toName)
        return false;

    SerialDevice *serialDev = mDevices.value(fromName);

    if(!serialDev || serialDev->isConnected() || mDevices.contains(toName))
        return false;

    mDevices.remove(fromName);
    mDevices.insert(toName, serialDev);

    mDevicesModel->sortItems();

    return true;
}

SerialDevicesModel *SerialManager::devicesModel() const
{
    return mDevicesModel;
}

static constexpr const quint8 handShakeReq[] =
{
    SerialManager::SerialCommands::NameRequest,
    's', 'i', 'm', 'r'
};


SerialManager::SerialManager(ModeManager *mgr)
    : QObject{mgr}
    , mModeMgr(mgr)
{
    mDevicesModel = new SerialDevicesModel(this);
}

SerialManager::~SerialManager()
{
    clear();

    delete mDevicesModel;
    mDevicesModel = nullptr;
}

void SerialManager::rescanPorts()
{
    bool needsRescan = false;
    for(const SerialDevice* dev : mDevices)
    {
        if(!dev->isConnected())
        {
            needsRescan = true;
            break;
        }
    }

    if(!needsRescan)
    {
        qDebug() << "All serial devices connected!";
        mRescanTimer.stop();
        return;
    }

    qDebug() << "Serial port scanning...";

    const QList<QSerialPortInfo> allPorts = QSerialPortInfo::availablePorts();

    for(const QSerialPortInfo& info : allPorts)
    {
        bool alreadyActive = false;
        for(const PendingPort &p : mPendingNamePorts)
        {
            if(p.serialPort->portName() == info.portName())
            {
                alreadyActive = true;
                break;
            }
        }

        if(!alreadyActive)
        {
            for(const SerialDevice* dev : mDevices)
            {
                if(dev->serialPort && dev->serialPort->portName() == info.portName())
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
            //qDebug() << serialPort->portName() << "ERROR:" << serialPort->error() << serialPort->errorString();
            delete serialPort;
            continue;
        }
        else
        {
            qDebug() << "Connected to:" << serialPort->portName();
        }

        connect(serialPort, &QSerialPort::readyRead,
                this, &SerialManager::onSerialRead);
        connect(serialPort, &QSerialPort::errorOccurred,
                this, &SerialManager::onSerialDisconnected);

        PendingPort p;
        p.serialPort = serialPort;
        p.firstConnect = QTime::currentTime();
        mPendingNamePorts.append(p);

        if(!mPendingTimer.isActive())
            mPendingTimer.start(500, this);

        // Init handshake
        writeMessageFlush(serialPort, handShakeReq, sizeof(handShakeReq));
    }

    // Schedule next rescan until all devices found
    scheduleRescan();
}

void SerialManager::clear()
{
    disconnectAllDevices();

    const auto devicesCopy = mDevices;
    for(SerialDevice *dev : devicesCopy)
        removeDevice(dev);

    Q_ASSERT(mDevices.isEmpty());
}

void SerialManager::disconnectAllDevices()
{
    for(SerialDevice* dev : mDevices)
    {
        dev->reset();
        dev->closeSerial();
    }

    for(PendingPort &p : mPendingNamePorts)
    {
        p.serialPort->close();
        delete p.serialPort;
        p.serialPort = nullptr;
    }

    mPendingNamePorts.clear();
    mPendingTimer.stop();

    mPingTimer.stop();

    mRescanTimer.stop();

    mDevicesModel->updateDeviceStatus();
}

void SerialManager::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mPendingTimer.timerId())
    {
        const QTime now = QTime::currentTime();
        for(auto pend = mPendingNamePorts.begin(); pend != mPendingNamePorts.end(); )
        {
            if(pend->firstConnect.msecsTo(now) > 5000 || pend->retryCount >= 5)
            {
                qDebug() << "Serial pending device timeout:" << pend->serialPort->portName();

                pend->serialPort->close();
                delete pend->serialPort;

                pend = mPendingNamePorts.erase(pend);
                continue;
            }

            // Retry handshake
            writeMessageFlush(pend->serialPort, handShakeReq, sizeof(handShakeReq));
            pend->retryCount++;

            pend++;
        }

        if(mPendingNamePorts.isEmpty())
            mPendingTimer.stop();

        return;
    }
    else if(e->timerId() == mPingTimer.timerId())
    {
        bool hasOpenPorts = false;

        const QTime now = QTime::currentTime();
        for(SerialDevice *dev : mDevices)
        {
            if(!dev->isConnected())
                continue;

            if(dev->lastPingReply.msecsTo(now) > 1000)
            {
                qDebug() << "Serial device timeout:" << dev->serialPort->portName() << dev->serialPort->objectName();

                dev->reset();
                dev->closeSerial();
                mDevicesModel->updateDeviceStatus();

                scheduleRescan();
                continue;
            }

            // Send ping
            hasOpenPorts = true;

            const quint8 msg = SerialCommands::Ping;
            writeMessageFlush(dev->serialPort, &msg, 1);
        }

        if(!hasOpenPorts)
            mPingTimer.stop();

        return;
    }
    else if(e->timerId() == mRescanTimer.timerId())
    {
        rescanPorts();
        return;
    }

    QObject::timerEvent(e);
}

void SerialManager::onSerialRead()
{
    QSerialPort *serialPort = qobject_cast<QSerialPort *>(sender());

    //qDebug() << "SERIAL READ:" << serialPort->portName() << serialPort->objectName() << serialPort->bytesAvailable();

    if(serialPort->objectName().isEmpty())
    {
        if(!checkSerialValid(serialPort))
            return;
    }

    SerialDevice *dev = getDevice(serialPort->objectName());

    while(serialPort->bytesAvailable() != 0)
    {
        bool reachedEnd = false;
        QByteArray msg = readMessage(serialPort, reachedEnd);
        if(reachedEnd)
            break;

        if(dev && !msg.isEmpty())
            dev->processMessage(msg);
    }
}

void SerialManager::onSerialDisconnected()
{
    QSerialPort *serialPort = qobject_cast<QSerialPort *>(sender());
    if(!serialPort)
    {
      qDebug() << "NULL SENDER" << sender();
      return;
    }
    qDebug() << "Serial disconnected:" << serialPort->portName() << serialPort->objectName() << serialPort->error() << serialPort->errorString();

    if(serialPort->error() == QSerialPort::TimeoutError)
    {
        // Try again
        writeMessageFlush(serialPort, handShakeReq, sizeof(handShakeReq));

        return;
    }

    if(serialPort->objectName().isEmpty())
    {
        for(auto pend = mPendingNamePorts.begin(); pend != mPendingNamePorts.end(); pend++)
        {
            if(pend->serialPort == serialPort)
            {
                mPendingNamePorts.erase(pend);
                break;
            }
        }

        if(mPendingNamePorts.isEmpty())
            mPendingTimer.stop();

        return;
    }

    SerialDevice *dev = mDevices.value(serialPort->objectName(), nullptr);
    Q_ASSERT(dev);

    dev->reset();
    dev->closeSerial();
    mDevicesModel->updateDeviceStatus();

    scheduleRescan();

    bool hasOpenPorts = false;
    for(const SerialDevice* otherDev : mDevices)
    {
        if(otherDev->isConnected())
        {
            hasOpenPorts = true;
            break;
        }
    }

    if(!hasOpenPorts)
        mPingTimer.stop();
}

bool SerialManager::checkSerialValid(QSerialPort *serialPort)
{
    bool reachedEnd = false;
    QByteArray reply = readMessage(serialPort, reachedEnd);

    if(reply.isEmpty() || reply.at(0) != SerialCommands::NameResponse)
        return false;

    const QString name = QString::fromUtf8(QByteArrayView(reply).sliced(1).trimmed());
    if(name.isEmpty())
        return false;

    for(auto pend = mPendingNamePorts.begin(); pend != mPendingNamePorts.end(); pend++)
    {
        if(pend->serialPort == serialPort)
        {
            mPendingNamePorts.erase(pend);
            break;
        }
    }

    if(mPendingNamePorts.isEmpty())
        mPendingTimer.stop();

    SerialDevice *dev = mDevices.value(name, nullptr);
    if(dev && !dev->isConnected())
    {
        qDebug() << "Serial device paired:" << serialPort->portName() << name;

        serialPort->setObjectName(name);
        dev->start(serialPort);
        mDevicesModel->updateDeviceStatus();

        if(!mPingTimer.isActive())
            mPingTimer.start(500, this);

        return true;
    }

    // Device is valid but we are not interested in it
    qDebug() << "Serial device discarded:" << serialPort->portName() << name;
    serialPort->close();
    delete serialPort;

    return false;
}

void SerialManager::scheduleRescan()
{
    if(!mRescanTimer.isActive())
        mRescanTimer.start(2000, this);
}

SerialDevice *SerialManager::getDevice(const QString &devName) const
{
    return mDevices.value(devName, nullptr);
}

SerialDevice *SerialManager::addDevice(const QString &devName)
{
    const QString trimmedName = devName.simplified();

    auto it = mDevices.constFind(trimmedName);
    if(it != mDevices.constEnd())
        return it.value();

    SerialDevice *serialDev = new SerialDevice(trimmedName);
    mDevices.insert(trimmedName, serialDev);

    mDevicesModel->addSerialDevice(serialDev);

    return serialDev;
}

void SerialManager::removeDevice(SerialDevice *serialDev)
{
    if(!serialDev)
        return;

    mDevices.remove(serialDev->getName());
    mDevicesModel->removeSerialDevice(serialDev);
    emit serialDeviceRemoved(serialDev);

    delete serialDev;
}
