/**
 * src/serial/serialdevice.h
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

#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QString>
#include <QTime>
#include <QHash>

class QSerialPort;

class RemoteCircuitBridge;

class SerialManager;

class SerialDevice
{
public:
    SerialDevice(const QString& name) : mName(name) {}
    ~SerialDevice();

    inline QString getName() const { return mName; }

    bool setName(const QString& newName, SerialManager *mgr);

    inline bool isConnected() const { return serialPort != nullptr; }

    void onOutputChanged(int outputId, int mode);

    inline bool isInputFree(int inputId) const
    {
        return !inputs.contains(inputId);
    }

    inline bool isOutputFree(int outputId) const
    {
        return !outputs.contains(outputId);
    }

    void addRemoteBridge(RemoteCircuitBridge *bridge);
    void removeRemoteBridge(RemoteCircuitBridge *bridge);

    bool changeRemoteBridgeInput(RemoteCircuitBridge *bridge,
                                 int oldValue, int newValue);

    bool changeRemoteBridgeOutput(RemoteCircuitBridge *bridge,
                                  int oldValue, int newValue);

private:
    friend class SerialManager;
    void reset();

    void closeSerial();

    void start(QSerialPort *serial);

    void processMessage(const QByteArray &msg);

private:
    QString mName;
    QSerialPort *serialPort = nullptr;

    QHash<int, RemoteCircuitBridge *> inputs;
    QHash<int, RemoteCircuitBridge *> outputs;
    QTime lastPingReply;
};

#endif // SERIALDEVICE_H
