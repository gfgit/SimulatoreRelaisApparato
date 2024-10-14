#include "relaispowernode.h"

#include "../abstractrelais.h"

RelaisPowerNode::RelaisPowerNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 2 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<AbstractCircuitNode::CableItem> RelaisPowerNode::getConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 || source.nodeContact != 1)
        return {};

    const NodeContact& side = mContacts[source.nodeContact];
    const NodeContact& otherSide = mContacts[source.nodeContact == 0 ? 1 : 0];

    QVector<AbstractCircuitNode::CableItem> result;
    result.reserve(side.cables.size() + otherSide.cables.size());

    for(const CableItem& item : side.cables)
    {
        if(item == source)
            continue;

        result.append(item);
    }

    // Close the circuit
    for(const CableItem& item : otherSide.cables)
    {
        result.append(item);
    }

    return result;
}

void RelaisPowerNode::addCircuit(ClosedCircuit *circuit)
{
    bool wasEmpty = mCircuits.isEmpty();

    AbstractCircuitNode::addCircuit(circuit);

    if(mRelais && wasEmpty && !mCircuits.isEmpty())
    {
        mRelais->powerNodeActivated(this);
    }
}

void RelaisPowerNode::removeCircuit(ClosedCircuit *circuit)
{
    bool hadCircuit = !mCircuits.isEmpty();

    AbstractCircuitNode::removeCircuit(circuit);

    if(mRelais && hadCircuit && mCircuits.isEmpty())
    {
        mRelais->powerNodeDeactivated(this);
    }
}
