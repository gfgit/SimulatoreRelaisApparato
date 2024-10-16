#include "circuitcable.h"

#include "abstractcircuitnode.h"

CircuitCable::CircuitCable(QObject *parent)
    : QObject{parent}
{

}

CircuitCable::~CircuitCable()
{
    Q_ASSERT(mFirstCableCirctuits.isEmpty());
    Q_ASSERT(mSecondCableCirctuits.isEmpty());

    // Detach all nodes
    if(mNodeA1.node)
    {
        AbstractCircuitNode::CableItem item;
        item.cable = this;
        item.cableSide = Side::A1;
        item.nodeContact = mNodeA1.nodeContact;
        mNodeA1.node->detachCable(item);
    }

    if(mNodeA2.node)
    {
        AbstractCircuitNode::CableItem item;
        item.cable = this;
        item.cableSide = Side::A2;
        item.nodeContact = mNodeA2.nodeContact;
        mNodeA2.node->detachCable(item);
    }

    if(mNodeB1.node)
    {
        AbstractCircuitNode::CableItem item;
        item.cable = this;
        item.cableSide = Side::B1;
        item.nodeContact = mNodeB1.nodeContact;
        mNodeB1.node->detachCable(item);
    }

    if(mNodeB2.node)
    {
        AbstractCircuitNode::CableItem item;
        item.cable = this;
        item.cableSide = Side::B2;
        item.nodeContact = mNodeB2.nodeContact;
        mNodeB2.node->detachCable(item);
    }
}

CircuitCable::Mode CircuitCable::mode() const
{
    return mMode;
}

void CircuitCable::setMode(Mode newMode)
{
    if (mMode == newMode)
        return;
    mMode = newMode;

    emit modeChanged(mMode);
}

void CircuitCable::addCircuit(ClosedCircuit *circuit, Side s)
{
    const Power oldPower = powered();

    if(s == Side::A1 || s == Side::B1)
    {
        if(mFirstCableCirctuits.contains(circuit))
            return; // A circuit may pass 2 times on same item
        mFirstCableCirctuits.append(circuit);
    }
    else
    {
        if(mSecondCableCirctuits.contains(circuit))
            return; // A circuit may pass 2 times on same item
        mSecondCableCirctuits.append(circuit);
    }

    const Power newPower = powered();
    if(oldPower != newPower)
        emit powerChanged(newPower);
}

void CircuitCable::removeCircuit(ClosedCircuit *circuit)
{
    const Power oldPower = powered();

    Q_ASSERT(mFirstCableCirctuits.contains(circuit)
             || mSecondCableCirctuits.contains(circuit));

    mFirstCableCirctuits.removeOne(circuit);
    mSecondCableCirctuits.removeOne(circuit);

    const Power newPower = powered();
    if(oldPower != newPower)
        emit powerChanged(newPower);
}

void CircuitCable::setNode(Side s, CableEnd node)
{
    switch (s)
    {
    case Side::A1:
        mNodeA1 = node;
        break;
    case Side::A2:
        mNodeA2 = node;
        break;
    case Side::B1:
        mNodeB1 = node;
        break;
    case Side::B2:
        mNodeB2 = node;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

CircuitCable::Power CircuitCable::powered()
{
    // Cable is powered if there are
    // closed circuits passing on it

    bool firstOn = !mFirstCableCirctuits.isEmpty();

    if(mSecondCableCirctuits.isEmpty())
    {
        return firstOn ? Power::FirstCable : Power::None;
    }

    return firstOn ? Power::BothOn : Power::SecondCable;
}
