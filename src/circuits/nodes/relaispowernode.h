/**
 * src/circuits/nodes/relaispowernode.h
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

#ifndef RELAISPOWERNODE_H
#define RELAISPOWERNODE_H

#include "abstractcircuitnode.h"

#include "../../enums/signalaspectcodes.h"

class AbstractRelais;

class RelaisPowerNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit RelaisPowerNode(ModeManager *mgr, QObject *parent = nullptr);
    ~RelaisPowerNode();

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences& items) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    static constexpr QLatin1String NodeType = QLatin1String("relais_power");
    QString nodeType() const override;

    bool tryFlipNode(bool forward) override;

    AbstractRelais *relais() const;
    void setRelais(AbstractRelais *newRelais);

    int delayUpMillis() const;
    void setDelayUpMillis(int newDelayUpMillis);

    int delayDownMillis() const;
    void setDelayDownMillis(int newDelayDownMillis);

    bool hasSecondConnector() const;
    void setHasSecondConnector(bool newHasSecondConnector);

    inline bool isTimeoutActive() const
    {
        return mPercentTimerId > 0;
    }

    inline double getTimeoutPercent() const
    {
        const double percent = std::max(mTimeoutPercentStatus[0],
                                        mTimeoutPercentStatus[1]);
        return qBound(0.0, percent, 1.0);
    }

    bool combinatorSecondCoil() const;
    void setCombinatorSecondCoil(bool newCombinatorSecondCoil);

signals:
    void relayChanged(AbstractRelais *r);
    void delaysChanged();

private slots:
    void onRelayTypeChanged();

protected:
    void timerEvent(QTimerEvent *e) override;

    void onCircuitFlagsChanged() override;

private:
    void activateRelay(int contact);
    void deactivateRelay(int contact);
    void stopTimer(int contact);
    void ensureTimeoutPercentTimer();
    void stopTimeoutPercentTimer();

    void updateDecoderState();
    void updateDiskRelayState();

private:
    // Settings
    AbstractRelais *mRelais = nullptr;

    int mDelayUpMillis = 0;
    int mDelayDownMillis = 0;
    bool mHasSecondConnector = false;
    bool mCombinatorSecondCoil = false;

    // State
    int mTimerIds[2] = {0, 0};
    int mPercentTimerId = 0;

    double mTimeoutPercentStatus[2] = {0, 0};
    bool wasGoingUp[2] = {true, true};
    bool mIsUp[2] = {false, false};

    bool skipDecoderUpdate = false;
    bool needsFlagUpdate = false;
    SignalAspectCode nextDetectedCode = SignalAspectCode::CodeAbsent;
};

#endif // RELAISPOWERNODE_H
