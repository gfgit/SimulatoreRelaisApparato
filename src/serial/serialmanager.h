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

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(ModeManager *mgr);
    ~SerialManager();

    void rescanPorts();

    void disconnectAllDevices();

    void onOutputChanged(qint64 deviceId, int outputId, int mode);

    void addRemoteBridge(RemoteCircuitBridge *bridge, const QString &devName);
    void removeRemoteBridge(RemoteCircuitBridge *bridge, const QString &devName);

    bool changeRemoteBridgeInput(RemoteCircuitBridge *bridge, const QString &devName,
                                  int oldValue, int newValue);
    bool changeRemoteBridgeOutput(RemoteCircuitBridge *bridge, const QString &devName,
                                  int oldValue, int newValue);

protected:
    void timerEvent(QTimerEvent *e) override;

private slots:
    void onSerialRead();
    void onSerialDisconnected();

private:
    bool checkSerialValid(QSerialPort *serialPort);

    void processMessage(qint64 deviceId, const QByteArray& msg);

    void scheduleRescan();

private:
    ModeManager *mModeMgr = nullptr;

    struct SerialDevice
    {
        void reset();

        void closeSerial();

        void start(QSerialPort *serial);

        inline bool isConnected() const { return serialPort != nullptr; }

        QSerialPort *serialPort = nullptr;

        QHash<int, RemoteCircuitBridge *> inputs;
        QHash<int, RemoteCircuitBridge *> outputs;
        QTime lastPingReply;
    };

    QHash<qint64, SerialDevice> mDevices;

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
