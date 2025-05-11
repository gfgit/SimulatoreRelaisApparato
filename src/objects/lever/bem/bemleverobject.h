/**
 * src/objects/lever/bem/bemleverobject.h
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

#ifndef BEMLEVEROBJECT_H
#define BEMLEVEROBJECT_H

#include "../../abstractsimulationobject.h"

enum class BEMLeverPositionMc
{
    Normal = 0,
    Middle1,
    Blocked,
    Middle2,
    Consensus,
    Middle3, // After this you go back to first
    NPositions
};

enum class BEMLeverPositionMr
{
    Normal = 0,
    Middle1,
    RequestConsensus,
    Middle2,
    SignalsAtStop,
    Middle3,
    ActivateFirstCatSignal,
    Middle4,
    ActivateBothSignals,
    Middle5, // After this you go back to first
    NPositions
};

class AbstractRelais;

class LeverInterface;
class BEMHandleInterface;
class ButtonInterface;

class BEMLeverObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    explicit BEMLeverObject(AbstractSimulationObjectModel *m);
    ~BEMLeverObject();

    static constexpr QLatin1String Type = QLatin1String("bem_lever");
    QString getType() const override;

protected:
    virtual void onInterfaceChanged(AbstractObjectInterface *iface,
                                    const QString &propName,
                                    const QVariant &value) override;

private slots:
    void onLiberationStateChanged();

private:
    void recalculateLockedRange();
    void fixBothInMiddlePosition();
    void updateArtLibButLock();
    void setArtLibBitLocked(bool lock);

    ButtonInterface *artificialLiberationBut() const;
    void setArtificialLiberationBut(ButtonInterface *newArtificialLiberationBut);

    AbstractRelais *liberationRelay() const;
    void setLiberationRelay(AbstractRelais *newLiberationRelay);

private:
    LeverInterface *leverInterface = nullptr;
    BEMHandleInterface *bemInterface = nullptr;

    AbstractRelais *mLiberationRelay = nullptr;
    ButtonInterface *mArtificialLiberationBut = nullptr;
};

#endif // BEMLeverObject_H
