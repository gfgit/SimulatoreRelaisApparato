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

    int delayUpSeconds() const;
    void setDelayUpSeconds(int newDelayUpSeconds);

    int delayDownSeconds() const;
    void setDelayDownSeconds(int newDelayDownSeconds);

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

private:
    void activateRelay(int contact);
    void deactivateRelay(int contact);
    void stopTimer(int contact);
    void ensureTimeoutPercentTimer();
    void stopTimeoutPercentTimer();

private:
    // Settings
    AbstractRelais *mRelais = nullptr;

    int mDelayUpSeconds = 0;
    int mDelayDownSeconds = 0;
    bool mHasSecondConnector = false;
    bool mCombinatorSecondCoil = false;

    // State
    int mTimerIds[2] = {0, 0};
    int mPercentTimerId = 0;

    double mTimeoutPercentStatus[2] = {0, 0};
    bool wasGoingUp[2] = {true, true};
    bool mIsUp[2] = {false, false};
};

#endif // RELAISPOWERNODE_H
