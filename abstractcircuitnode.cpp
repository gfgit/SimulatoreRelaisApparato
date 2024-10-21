#include "abstractcircuitnode.h"

#include "closedcircuit.h"

#include <QJsonObject>

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
        detachCable(i);
    }
}

void AbstractCircuitNode::addCircuit(ClosedCircuit *circuit)
{
    // A circuit may pass 2 times on same node
    // But we add it only once
    if(mCircuits.contains(circuit))
        return;

    mCircuits.append(circuit);

    bool updateNeeded = false;

    const auto items = circuit->getNode(this);
    for(const ClosedCircuit::NodeItem& item : items)
    {
        int &fromCount = mContacts[item.fromContact].circuitsCount;
        int &toCount = mContacts[item.toContact].circuitsCount;

        fromCount++;
        toCount++;

        if(fromCount == 1 || toCount == 1)
            updateNeeded = true;
    }

    if(mCircuits.size() == 1 || updateNeeded)
        emit circuitsChanged();
}

void AbstractCircuitNode::removeCircuit(ClosedCircuit *circuit)
{
    Q_ASSERT(mCircuits.contains(circuit));

    bool updateNeeded = false;

    const auto items = circuit->getNode(this);
    for(const ClosedCircuit::NodeItem& item : items)
    {
        int &fromCount = mContacts[item.fromContact].circuitsCount;
        int &toCount = mContacts[item.toContact].circuitsCount;

        Q_ASSERT(fromCount > 0 && toCount > 0);

        fromCount--;
        toCount--;

        if(fromCount == 0 || toCount == 0)
            updateNeeded = true;
    }

    mCircuits.removeOne(circuit);

    if(mCircuits.size() == 0 || updateNeeded)
        emit circuitsChanged();
}

void AbstractCircuitNode::attachCable(const CableItem& item)
{
    // Either contact if free or we are adding poles to same cable
    Q_ASSERT(item.cable.cable->getNode(item.cable.side).node == this || !item.cable.cable->getNode(item.cable.side).node);
    Q_ASSERT(item.nodeContact < mContacts.size());
    Q_ASSERT(mContacts.at(item.nodeContact).cable == item.cable.cable || !mContacts.at(item.nodeContact).cable);
    Q_ASSERT(mContacts.at(item.nodeContact).getType(item.cable.pole) == ContactType::NotConnected);

    NodeContact& contact = mContacts[item.nodeContact];
    if(!contact.cable)
    {
        // Connect new cable
        contact.cable = item.cable.cable;
        contact.cableSide = item.cable.side;
        contact.setType(item.cable.pole, ContactType::Connected);

        CircuitCable::CableEnd cableEnd;
        cableEnd.node = this;
        cableEnd.nodeContact = item.nodeContact;
        item.cable.cable->setNode(item.cable.side, cableEnd);
    }
    else
    {
        // Add a pole
        contact.setType(item.cable.pole, ContactType::Connected);
    }
}

void AbstractCircuitNode::detachCable(const CableItem &item)
{
    Q_ASSERT(item.cable.cable->getNode(item.cable.side).node == this);
    Q_ASSERT(item.nodeContact < mContacts.size());
    Q_ASSERT(mContacts.at(item.nodeContact).cable == item.cable.cable);
    Q_ASSERT(mContacts.at(item.nodeContact).getType(item.cable.pole) != ContactType::NotConnected);

    NodeContact& contact = mContacts[item.nodeContact];

    // Check other pole
    if(contact.getType(~item.cable.pole) != ContactType::Connected)
    {
        // Other pole is not connected, remove cable
        contact.cable = nullptr;
        item.cable.cable->setNode(item.cable.side, {});
        contact.setType(item.cable.pole, ContactType::NotConnected);
    }
    else
    {
        // Keep other pole
        contact.setType(item.cable.pole, ContactType::NotConnected);
    }
}

bool AbstractCircuitNode::loadFromJSON(const QJsonObject &obj)
{
    if(obj.value("type") != nodeType())
        return false;

    setObjectName(obj.value("name").toString());
    return true;
}

void AbstractCircuitNode::saveToJSON(QJsonObject &obj) const
{
    obj["type"] = nodeType();
    obj["name"] = objectName();
}

void AbstractCircuitNode::detachCable(int contactIdx)
{
    NodeContact& contact = mContacts[contactIdx];
    if(contact.cable)
    {
        contact.cable->setNode(contact.cableSide, {});
        contact.type1 = contact.type2 = ContactType::NotConnected;
        contact.cable = nullptr;
    }
}
