#ifndef CLOSEDCIRCUIT_H
#define CLOSEDCIRCUIT_H

#include "circuitcable.h"
#include "abstractcircuitnode.h"

class PowerSourceNode;

class ClosedCircuit : public QObject
{
    Q_OBJECT
public:
    struct CableItem
    {
        CircuitCable *cable = nullptr;
        CircuitCable::Side fromSide = CircuitCable::Side::A1;
    };

    struct NodeItem
    {
        AbstractCircuitNode *node = nullptr;
        int fromContact = 0;
        int toContact = 0;
    };

    struct Item
    {
        CableItem *cable = nullptr;
        NodeItem *node = nullptr;
        bool isNode = false;
    };

    explicit ClosedCircuit(QObject *parent = nullptr);

    void enableCircuit();
    void disableCircuit();

    static createCircuitsFromPowerNode(PowerSourceNode *source);

private:
    static passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item>& items, int depth);

public:
    QVector<Item> mItems;
    bool enabled = false;
};

#endif // CLOSEDCIRCUIT_H
