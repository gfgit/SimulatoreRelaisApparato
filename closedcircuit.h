#ifndef CLOSEDCIRCUIT_H
#define CLOSEDCIRCUIT_H

#include "circuitcable.h"
#include "abstractcircuitnode.h"

class PowerSourceNode;

class ClosedCircuit : public QObject
{
    Q_OBJECT
public:
    struct NodeItem
    {
        AbstractCircuitNode *node = nullptr;
        int fromContact = 0;
        int toContact = 0;
        CircuitCable::Pole fromPole = CircuitCable::Pole::First;
        CircuitCable::Pole toPole = CircuitCable::Pole::First;
    };

    struct Item
    {
        CircuitCable::CableContact cable;
        NodeItem node;
        bool isNode = false;
    };

    explicit ClosedCircuit(QObject *parent = nullptr);

    void enableCircuit();
    void disableCircuit();

    QVector<NodeItem> getNode(AbstractCircuitNode *node) const;

    static void createCircuitsFromPowerNode(PowerSourceNode *source);
    static void createCircuitsFromOtherNode(AbstractCircuitNode *source, const QVector<AbstractCircuitNode::NodeContact> &contacts);

private:
    static void passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item>& items, int depth);

    static void searchPowerSource(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth);

    static void continueCircuitPassingLastNode(const QVector<Item> &items, int depth);

private:
    QVector<Item> mItems;
    bool enabled = false;
    bool isDisabling = false;
};

#endif // CLOSEDCIRCUIT_H
