/**
 * src/circuits/nodes/remotecablecircuitnode.h
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
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

#ifndef REMOTE_CABLE_CIRCUIT_NODE_H
#define REMOTE_CABLE_CIRCUIT_NODE_H

#include "abstractcircuitnode.h"

class RemoteCircuitBridge;

/*!
 * \brief The RemoteCableCircuitNode class
 *
 * This node simulates a cable continuing to another circuit
 * which is not visually connected in the Circuit Scene.
 *
 * On the other circuit there is another twin \a RemoteCableCircuitNode
 * which acts as a controlled power source to make current keep going.
 */
class RemoteCableCircuitNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    enum class Mode
    {
        None = 0,
        SendCurrentOpen,
        SendCurrentWaitClosed,
        SendCurrentClosed,
        ReceiveCurrentOpen,
        ReceiveCurrentWaitClosed,
        ReceiveCurrentClosed
    };

    explicit RemoteCableCircuitNode(ModeManager *mgr, QObject *parent = nullptr);
    ~RemoteCableCircuitNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items) override;
    void partialRemoveCircuit(ElectricCircuit *circuit, const NodeOccurences &items) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("remove_cable_node");
    QString nodeType() const override;

    bool isSourceNode(bool onlyCurrentState) const override;

    bool sourceDoNotCloseCircuits() const override;

    bool isSourceEnabled() const override;
    void setSourceEnabled(bool newEnabled) override;

    Mode mode() const;
    void setMode(Mode newMode);

    static inline bool isReceiveMode(Mode m)
    {
        switch (m)
        {
        case Mode::ReceiveCurrentOpen:
        case Mode::ReceiveCurrentWaitClosed:
        case Mode::ReceiveCurrentClosed:
            return true;
        default:
            break;
        }

        return false;
    }

    static inline bool isSendMode(Mode m)
    {
        switch (m)
        {
        case Mode::SendCurrentOpen:
        case Mode::SendCurrentWaitClosed:
        case Mode::SendCurrentClosed:
            return true;
        default:
            break;
        }

        return false;
    }

    inline bool isReceiveSide() const
    {
        return isReceiveMode(mode());
    }

    inline bool isSendSide() const
    {
        return isSendMode(mode());
    }

    RemoteCircuitBridge *remote() const;
    void setRemote(RemoteCircuitBridge *newRemote, bool autoSwap = true);

    bool isNodeA() const;
    void setIsNodeA(bool newIsNodeA);

    inline CircuitPole getSendPole() const { return mSendPole; }

    QString getDescription() const;

signals:
    void modeChanged(Mode newMode);

private:
    friend class RemoteCircuitBridge;
    void onPeerModeChanged(Mode peerMode, CircuitPole peerSendPole);

    void scheduleStateRefresh();
    void refreshState();

private:
    RemoteCircuitBridge *mRemote = nullptr;
    bool mIsNodeA = true;
    bool mIsEnabled = false;

    int mFakeClosedCircuitsCount = 0;
    bool mStateDirty = false;
    Mode mMode = Mode::None;
    Mode mLastPeerMode = Mode::None;

    CircuitPole mSendPole = CircuitPole::First;
    CircuitPole mRecvPole = CircuitPole::First;
};

#endif // REMOTE_CABLE_CIRCUIT_NODE_H
