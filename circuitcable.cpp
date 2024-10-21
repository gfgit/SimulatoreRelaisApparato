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
    if(mNodeA.node)
    {
        CableItem item;
        item.cable.cable = this;
        item.cable.side = CableSide::A;
        item.nodeContact = mNodeA.nodeContact;

        item.cable.pole = CircuitPole::First;
        if(mNodeA.node->getContactType(mNodeA.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeA.node->detachCable(item);
        }

        item.cable.pole = CircuitPole::Second;
        if(mNodeA.node->getContactType(mNodeA.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeA.node->detachCable(item);
        }
    }

    if(mNodeB.node)
    {
        CableItem item;
        item.cable.cable = this;
        item.cable.side = CableSide::B;
        item.nodeContact = mNodeB.nodeContact;

        item.cable.pole = CircuitPole::First;
        if(mNodeB.node->getContactType(mNodeB.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeB.node->detachCable(item);
        }

        item.cable.pole = CircuitPole::Second;
        if(mNodeB.node->getContactType(mNodeB.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeB.node->detachCable(item);
        }
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

void CircuitCable::addCircuit(ElectricCircuit *circuit, CircuitPole pole)
{
    const Power oldPower = powered();

    if(pole == CircuitPole::First)
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

void CircuitCable::removeCircuit(ElectricCircuit *circuit)
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

void CircuitCable::setNode(CableSide s, CableEnd node)
{
    switch (s)
    {
    case CableSide::A:
        mNodeA = node;
        break;
    case CableSide::B:
        mNodeB = node;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    emit nodesChanged();
}

CircuitCable::Power CircuitCable::powered()
{
    // Cable is powered if there are
    // closed circuits passing on it

    const bool firstOn = !mFirstCableCirctuits.isEmpty();

    if(mSecondCableCirctuits.isEmpty())
    {
        return firstOn ? Power::FirstCable : Power::None;
    }

    return firstOn ? Power::BothOn : Power::SecondCable;
}
