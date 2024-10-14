#include "abstractcircuitnode.h"

AbstractCircuitNode::AbstractCircuitNode(QObject *parent)
    : QObject{parent}
{

}

void AbstractCircuitNode::addCircuit(ClosedCircuit *circuit)
{
    // A circuit may pass 2 times on same node
    // But we add it only once
    if(mCircuits.contains(circuit))
        return;

    mCircuits.append(circuit);

    if(mCircuits.size() == 1)
        emit circuitsChanged();
}

void AbstractCircuitNode::removeCircuit(ClosedCircuit *circuit)
{
    Q_ASSERT(mCircuits.contains(circuit));
    mCircuits.removeOne(circuit);

    if(mCircuits.size() == 0)
        emit circuitsChanged();
}

void AbstractCircuitNode::attachCable(CableItem item)
{
    Q_ASSERT(!item.cable->getNode(item.cableSide).node);
    Q_ASSERT(item.nodeContact < mContacts.size());

    NodeContact& contact = mContacts[item.nodeContact];
    contact.cables.append(item);

    CircuitCable::CableEnd cableEnd;
    cableEnd.node = this;
    cableEnd.nodeContact = item.nodeContact;
    item.cable->setNode(item.cableSide, cableEnd);
}

void AbstractCircuitNode::detachCable(CableItem item)
{
    Q_ASSERT(item.cable->getNode(item.cableSide).node == this);
    Q_ASSERT(item.nodeContact < mContacts.size());

    NodeContact& contact = mContacts[item.nodeContact];
    contact.cables.removeOne(item);

    item.cable->setNode(item.cableSide, {});
}
