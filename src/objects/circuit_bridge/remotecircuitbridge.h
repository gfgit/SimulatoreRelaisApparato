/**
 * src/objects/circuit_bridge/remotecircuitbridge.h
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

#ifndef REMOTE_CIRCUIT_BRIDGE_H
#define REMOTE_CIRCUIT_BRIDGE_H

#include "../abstractsimulationobject.h"

class RemoteCableCircuitNode;

class RemoteSession;

class RemoteCircuitBridge : public AbstractSimulationObject
{
    Q_OBJECT
public:
    explicit RemoteCircuitBridge(AbstractSimulationObjectModel *m);
    ~RemoteCircuitBridge();

    static constexpr QLatin1String Type = QLatin1String("circuit_bridge");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject& obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject& obj) const override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    RemoteCableCircuitNode *getNode(bool isA) const;

    inline QString getNodeDescription(bool isA) const
    {
        return isA ? mNodeDescriptionA : mNodeDescriptionB;
    }

    void setNodeDescription(bool isA, const QString& newDescr);

    // Connects to remote session or to serial device
    bool isRemote() const;

    // Remote Session
    QString remoteSessionName() const;

    inline RemoteSession *getRemoteSession() const
    {
        return mRemoteSession;
    }

    bool setRemoteSession(RemoteSession *remoteSession);

    // TODO: make private
    void onRemoteNodeModeChanged(qint8 mode, qint8 pole, qint8 replyToMode);
    void onRemoteDisconnected();
    void onRemoteStarted();

    // Either local name or peer custom name
    QString peerNodeName() const;

    inline QString peerNodeCustomName() const
    {
        return mPeerNodeCustomName;
    }

    bool setPeerNodeCustomName(const QString &newPeerNodeName);

    // Serial Device
    QString getDeviceName() const;
    bool setDeviceName(const QString &name);

    int serialInputId() const;
    void setSerialInputId(int newSerialInputId);

    int serialOutputId() const;
    void setSerialOutputId(int newSerialOutputId);

private slots:
    void onNameChanged(AbstractSimulationObject *,
                       const QString& newName, const QString& oldName);

private:
    void setIsRemote(bool val);

    friend class RemoteCableCircuitNode;
    void setNode(RemoteCableCircuitNode *newNode, bool isA);

    void onLocalNodeModeChanged(RemoteCableCircuitNode *node);

    friend class SerialManager;
    void onSerialInputMode(int mode);

private:
    RemoteCableCircuitNode *mNodeA = nullptr;
    RemoteCableCircuitNode *mNodeB = nullptr;


    QString mNodeDescriptionA;
    QString mNodeDescriptionB;

    // Remote Session
    friend class RemoteManager;
    friend class RemoteSession;
    RemoteSession *mRemoteSession = nullptr;
    QString mPeerNodeCustomName;
    size_t mPeerNodeId = 0;

    // Serial Device
    QString mSerialName;
    qint64 mSerialNameId = 0;
    int mSerialInputId = 0;
    int mSerialOutputId = 0;
};

#endif // REMOTE_CIRCUIT_BRIDGE_H
