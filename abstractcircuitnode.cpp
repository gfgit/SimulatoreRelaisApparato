#include "abstractcircuitnode.h"

AbstractCircuitNode::AbstractCircuitNode(QObject *parent)
    : QObject{parent}
{

}

AbstractCircuitNode::~AbstractCircuitNode()
{
    Q_ASSERT(mCircuits.isEmpty());

    // Detach all contacts
    for(int i = 0; i < mContacts.size(); i++)
    {
        if(mContacts.at(i).item.cable)
            detachCable(mContacts.at(i).item);
    }
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

    if(mContacts.at(item.nodeContact).item.cable)
        return; // Already connected

    NodeContact& contact = mContacts[item.nodeContact];
    contact.item = item;

    CircuitCable::CableEnd cableEnd;
    cableEnd.node = this;
    cableEnd.nodeContact = item.nodeContact;
    item.cable->setNode(item.cableSide, cableEnd);
}

void AbstractCircuitNode::detachCable(CableItem item)
{
    Q_ASSERT(item.cable->getNode(item.cableSide).node == this);
    Q_ASSERT(item.nodeContact < mContacts.size());
    Q_ASSERT(mContacts.at(item.nodeContact).item == item);

    NodeContact& contact = mContacts[item.nodeContact];
    contact.item.cable = nullptr;

    item.cable->setNode(item.cableSide, {});
}
