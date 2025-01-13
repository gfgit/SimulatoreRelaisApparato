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

    void setRemote(bool val);
    void setRemoteSessionName(const QString& name);

    inline bool isRemote() const { return mIsRemote; }

    void onRemoteSessionRenamed(const QString& toName);

private:
    friend class RemoteCableCircuitNode;
    void setNode(RemoteCableCircuitNode *newNode, bool isA);

    void onNodeModeChanged(RemoteCableCircuitNode *node);

private:
    RemoteCableCircuitNode *mNodeA = nullptr;
    RemoteCableCircuitNode *mNodeB = nullptr;

    QString mNodeDescriptionA;
    QString mNodeDescriptionB;

    bool mIsRemote = false;
    QString mPeerSession;
    QString mPeerNodeName;

    size_t mPeerSessionId = 0;
    size_t mPeerNodeId = 0;
};

#endif // REMOTE_CIRCUIT_BRIDGE_H
