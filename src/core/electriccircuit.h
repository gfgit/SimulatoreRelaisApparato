/**
 * src/core/electriccircuit.h
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

#ifndef ELECTRICCIRCUIT_H
#define ELECTRICCIRCUIT_H

#include "../enums/cabletypes.h"
#include "../enums/circuittypes.h"

class AbstractCircuitNode;
class PowerSourceNode;

class ElectricCircuit
{
public:
    struct Item
    {
        CableContact cable;
        NodeItem node;
        bool isNode = false;
    };

    explicit ElectricCircuit();

    void enableCircuit();
    void disableOrTerminate(AbstractCircuitNode *node);
    void terminateHere(AbstractCircuitNode *goalNode, QVector<ElectricCircuit *>& deduplacteList);

    inline CircuitType type() const { return mType; }

    NodeOccurences getNode(AbstractCircuitNode *node) const;

    bool isLastNode(AbstractCircuitNode *node) const;

    PowerSourceNode *getSource() const;

    static void createCircuitsFromPowerNode(PowerSourceNode *source);
    static void createCircuitsFromOtherNode(AbstractCircuitNode *node);

    static void tryReachNextOpenCircuit(AbstractCircuitNode *goalNode,
                                        int nodeContact,
                                        CircuitPole pole);

    static void defaultReachNextOpenCircuit(AbstractCircuitNode *goalNode);

private:
    bool tryReachOpen(AbstractCircuitNode *goalNode);

    static bool passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item>& items, int depth);

    static void searchNodeWithOpenCircuits(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth);

    static void extendOpenCircuits(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items);

private:
    QVector<Item> mItems;
    bool enabled = false;
    bool isDisabling = false;
    CircuitType mType = CircuitType::Open;
};

constexpr bool operator ==(const ElectricCircuit::Item& lhs, const ElectricCircuit::Item& rhs)
{
    if(lhs.isNode != rhs.isNode)
        return false;

    if(lhs.isNode)
    {
        return lhs.node == rhs.node;
    }

    return lhs.cable == rhs.cable;
}

#endif // ELECTRICCIRCUIT_H
