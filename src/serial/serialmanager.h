/**
 * src/serial/serialmanager.h
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

#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>

#include <QHash>

#include <QTime>
#include <QBasicTimer>

class QSerialPort;

class RemoteCircuitBridge;

class ModeManager;

class SerialDevice;
class SerialDevicesModel;

class SerialManager : public QObject
{
    Q_OBJECT
public:
    enum SerialCommands
    {
        NameRequest  = 0x04,
        NameResponse = 0x05,
        OutputChange = 0x06,
        InputChange  = 0x07,
        PauseExecution  = 0x08,
        ResumeExecution = 0x09,
        // 0x10 is escape character
        Ping = 0x11,
        PingResponse = 0x12
    };

    explicit SerialManager(ModeManager *mgr);
    ~SerialManager();

    void rescanPorts();

    void clear();
    void disconnectAllDevices();

    SerialDevice *getDevice(const QString& devName) const;
    SerialDevice *addDevice(const QString& devName);
    void removeDevice(SerialDevice *serialDev);

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    SerialDevicesModel *devicesModel() const;

signals:
    void serialDeviceRemoved(SerialDevice *dev);

protected:
    void timerEvent(QTimerEvent *e) override;

private slots:
    void onSerialRead();
    void onSerialDisconnected();

private:
    bool checkSerialValid(QSerialPort *serialPort);

    void scheduleRescan();

    friend class SerialDevice;
    static void writeMessageFlush(QSerialPort *dev, const quint8 *data, int size);

    bool renameDevice(const QString &fromName, const QString &toName);

private:
    ModeManager *mModeMgr = nullptr;
    SerialDevicesModel *mDevicesModel = nullptr;

    QHash<QString, SerialDevice *> mDevices;

    struct PendingPort
    {
        QSerialPort *serialPort = nullptr;
        QTime firstConnect;
        int retryCount = 0;
    };

    QVector<PendingPort> mPendingNamePorts;

    QBasicTimer mPendingTimer;
    QBasicTimer mPingTimer;
    QBasicTimer mRescanTimer;
};

#endif // SERIALMANAGER_H
