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

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    void rescanPorts();

    void disconnectAllDevices();

    void onOutputChanged(qint64 deviceId, int outputId, int mode);

    void addRemoteBridge(RemoteCircuitBridge *bridge, const QString &devName);
    void removeRemoteBridge(RemoteCircuitBridge *bridge, const QString &devName);

protected:
    void timerEvent(QTimerEvent *e) override;

private slots:
    void onSerialRead();
    void onSerialDisconnected();

    bool checkSerialValid(QSerialPort *serialPort);

private:
    void onInputChanged(const QString& name, int inputId, int mode);

private:
    struct SerialDevice
    {
        QSerialPort *serialPort = nullptr;

        void reset();
        void start();

        QHash<int, RemoteCircuitBridge *> inputs;
        QHash<int, RemoteCircuitBridge *> outputs;
    };

    QHash<qint64, SerialDevice> mDevices;
    QVector<std::pair<QTime, QSerialPort *>> mPendingNamePorts;

    QBasicTimer mPendingClearTimer;
};

#endif // SERIALMANAGER_H
