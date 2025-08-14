/**
 * src/network/traintastic-simulator/traintasticsimmanager.h
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

#ifndef TRAINTASTICSIMMANAGER_H
#define TRAINTASTICSIMMANAGER_H

#include <QObject>
#include <QHash>

#include "protocol.hpp"

class QTcpSocket;

class TraintasticSensorObj;

class TraintasticSimManager : public QObject
{
    Q_OBJECT
public:
    explicit TraintasticSimManager(QObject *parent = nullptr);
    ~TraintasticSimManager();

    bool isConnected() const;

    void enableConnection(bool val);

signals:
    void stateChanged();

private slots:
    void onSocketError();

    void onReadyRead();

private:
    void receive(const SimulatorProtocol::Message &message);

    friend class TraintasticSensorObj;
    bool setSensorChannel(TraintasticSensorObj *obj, int newChannel);
    bool setSensorAddress(TraintasticSensorObj *obj, int newAddress);

    void setSensorsOff();

private:
    QTcpSocket *mSocket = nullptr;
    qint64 m_readBufferOffset = 0;

    static constexpr int BufSize = 100;
    std::byte m_readBuffer[BufSize];

    // By channel and then by address
    QHash<int, QHash<int, TraintasticSensorObj *>> mSensors;
    QHash<int, QHash<int, TraintasticSensorObj *>> mTurnoutSensors;
};

#endif // TRAINTASTICSIMMANAGER_H
