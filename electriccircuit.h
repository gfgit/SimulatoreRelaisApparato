#ifndef ELECTRICCIRCUIT_H
#define ELECTRICCIRCUIT_H

#include "enums/circuittypes.h"
#include "abstractcircuitnode.h"

class PowerSourceNode;

class ElectricCircuit : public QObject
{
    Q_OBJECT
public:
    struct NodeItem
    {
        AbstractCircuitNode *node = nullptr;
        int fromContact = 0;
        int toContact = 0;
        CircuitPole fromPole = CircuitPole::First;
        CircuitPole toPole = CircuitPole::First;
    };

    struct Item
    {
        CableContact cable;
        NodeItem node;
        bool isNode = false;
    };

    explicit ElectricCircuit(QObject *parent = nullptr);

    void enableCircuit();
    void disableCircuit();

    inline CircuitType type() const { return mType; }

    QVector<NodeItem> getNode(AbstractCircuitNode *node) const;

    bool isLastNode(AbstractCircuitNode *node) const;

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
    CircuitType mType = CircuitType::Open;
};

#endif // ELECTRICCIRCUIT_H
