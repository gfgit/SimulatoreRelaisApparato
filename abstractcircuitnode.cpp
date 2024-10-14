#include "abstractcircuitnode.h"

AbstractCircuitNode::AbstractCircuitNode(QObject *parent)
    : QObject{parent}
{

}

void AbstractCircuitNode::addCircuit(ClosedCircuit *circuit)
{
    Q_ASSERT(!mCircuits.contains(circuit));
    mCircuits.append(circuit);

    emit circuitsChanged();
}

void AbstractCircuitNode::removeCircuit(ClosedCircuit *circuit)
{
    Q_ASSERT(mCircuits.contains(circuit));
    mCircuits.removeOne(circuit);

    emit circuitsChanged();
}

void AbstractCircuitNode::attachCable(CableItem item)
{
    Q_ASSERT(!item.cable->getNode(item.cableSide).node);
    Q_ASSERT(item.nodeContact < mContacts.size());

    NodeContact& contact = mContacts[item.nodeContact];
    contact.cables.append(item);
    item.cable->setNode(item.cableSide, {});
}

void AbstractCircuitNode::detachCable(CableItem item)
{
    Q_ASSERT(item.cable->getNode(item.cableSide).node == this);
    Q_ASSERT(item.nodeContact < mContacts.size());

    NodeContact& contact = mContacts[item.nodeContact];
    contact.cables.removeOne(item);

    CircuitCable::CableEnd cableEnd;
    cableEnd.node = this;
    cableEnd.nodeContact = item.nodeContact;
    item.cable->setNode(item.cableSide, cableEnd);
}
