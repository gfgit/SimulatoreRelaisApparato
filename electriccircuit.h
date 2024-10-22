#ifndef ELECTRICCIRCUIT_H
#define ELECTRICCIRCUIT_H

#include "enums/circuittypes.h"
#include "abstractcircuitnode.h"

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

private:
    bool tryReachOpen(AbstractCircuitNode *goalNode);

    static bool passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item>& items, int depth);

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
