/**
 * src/circuits/nodes/circuitcable.h
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

#ifndef CIRCUITCABLE_H
#define CIRCUITCABLE_H

#include <QObject>
#include <QVector>

#include "../../enums/circuittypes.h"
#include "../../enums/cabletypes.h"

class ElectricCircuit;
class AbstractCircuitNode;

class ModeManager;

class CircuitCable : public QObject
{
    Q_OBJECT
public:

    enum class Mode
    {
        Unifilar = 0,
        Bifilar1,
        Bifilar2,
        BifilarBoth
    };

    CircuitCable(ModeManager *mgr, QObject *parent = nullptr);
    ~CircuitCable();

    Mode mode() const;
    void setMode(Mode newMode);

    CablePower powered() const;
    CircuitFlags getFlags() const;

    void addCircuit(ElectricCircuit *circuit, CircuitPole pole);
    void removeCircuit(ElectricCircuit *circuit);

    inline CableEnd getNode(CableSide s) const
    {
        switch (s)
        {
        case CableSide::A:
            return mNodeA;
        case CableSide::B:
            return mNodeB;
        default:
            break;
        }

        Q_UNREACHABLE();
        return {};
    }

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    void updateCircuitFlags(CircuitType type, CircuitPole pole);

signals:
    void modeChanged(Mode m);
    void powerChanged(CablePower p);
    void nodesChanged();

private:
    friend class AbstractCircuitNode;
    void setNode(CableSide s, CableEnd node);

private:
    ModeManager *mModeMgr;

    Mode mMode = Mode::Unifilar;

    typedef QVector<ElectricCircuit *> CircuitList;

    inline CircuitList& getCircuits(CircuitType type, CircuitPole pole)
    {
        if(pole == CircuitPole::First)
            return type == CircuitType::Closed ?
                        mFirstPoleCirctuitsClosed :
                        mFirstPoleCirctuitsOpen;

        return type == CircuitType::Closed ?
                    mSecondPoleCirctuitsClosed :
                    mSecondPoleCirctuitsOpen;
    }

    inline const CircuitList& getCircuits(CircuitType type, CircuitPole pole) const
    {
        if(pole == CircuitPole::First)
            return type == CircuitType::Closed ?
                        mFirstPoleCirctuitsClosed :
                        mFirstPoleCirctuitsOpen;

        return type == CircuitType::Closed ?
                    mSecondPoleCirctuitsClosed :
                    mSecondPoleCirctuitsOpen;
    }

    inline CircuitFlags& getFlags(CircuitType type, CircuitPole pole)
    {
        if(pole == CircuitPole::First)
            return type == CircuitType::Closed ?
                        mFlagsFirstClosed :
                        mFlagsFirstOpen;

        return type == CircuitType::Closed ?
                    mFlagsSecondClosed :
                    mFlagsSecondOpen;
    }

    inline CircuitFlags getFlags(CircuitType type, CircuitPole pole) const
    {
        if(pole == CircuitPole::First)
            return type == CircuitType::Closed ?
                        mFlagsFirstClosed :
                        mFlagsFirstOpen;

        return type == CircuitType::Closed ?
                    mFlagsSecondClosed :
                    mFlagsSecondOpen;
    }

    CircuitList mFirstPoleCirctuitsClosed;
    CircuitList mSecondPoleCirctuitsClosed;
    CircuitList mFirstPoleCirctuitsOpen;
    CircuitList mSecondPoleCirctuitsOpen;

    CircuitFlags mFlagsFirstClosed = CircuitFlags::None;
    CircuitFlags mFlagsSecondClosed = CircuitFlags::None;
    CircuitFlags mFlagsFirstOpen = CircuitFlags::None;
    CircuitFlags mFlagsSecondOpen = CircuitFlags::None;
    int mCircuitsWithFlags = 0;

    CableEnd mNodeA;
    CableEnd mNodeB;
};

#endif // CIRCUITCABLE_H
