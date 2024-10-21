#include "aceibuttonnode.h"

#include "../closedcircuit.h"

#include <QJsonObject>

ACEIButtonNode::ACEIButtonNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 3 sides
    mContacts.append(NodeContact()); // Common
    mContacts.append(NodeContact()); // Pressed
    mContacts.append(NodeContact()); // Normal
}

ACEIButtonNode::~ACEIButtonNode()
{

}

QVector<AbstractCircuitNode::CableItem> ACEIButtonNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact >= getContactCount()))
        return {};

    int otherContactIdx = -1;
    int otherContactIdx2 = -1;

    const NodeContact& sourceContact = mContacts.at(source.nodeContact);
    if(sourceContact.getType(source.cable.pole) == ContactType::Passthrough &&
            (source.nodeContact == 0 || source.nodeContact == 2))
    {
        // Pass to other contact straight
        otherContactIdx = source.nodeContact == 0 ? 2 : 0;
    }
    else if(mState != State::Extracted)
    {
        switch (source.nodeContact)
        {
        case 0:
            otherContactIdx = 2; // Normal is always connected
            if(mState == State::Pressed)
                otherContactIdx2 = 1;
            break;
        case 1:
            if(mState == State::Pressed)
            {
                otherContactIdx = 0;
                otherContactIdx2 = 2;
            }
            break;
        case 2:
            otherContactIdx = 0; // Common is always connected
            if(mState == State::Pressed)
                otherContactIdx2 = 1;
            break;
        default:
            break;
        }
    }

    QVector<AbstractCircuitNode::CableItem> result;

    if(otherContactIdx != -1)
    {
        const NodeContact& otherContact = mContacts.at(otherContactIdx);
        CableItem other;
        other.cable.cable = otherContact.cable;
        other.cable.side = otherContact.cableSide;
        other.cable.pole = source.cable.pole;
        other.nodeContact = otherContactIdx;

        if(other.cable.cable)
            result.append(other);
    }

    if(otherContactIdx2 != -1)
    {
        const NodeContact& otherContact = mContacts.at(otherContactIdx2);
        CableItem other;
        other.cable.cable = otherContact.cable;
        other.cable.side = otherContact.cableSide;
        other.cable.pole = source.cable.pole;
        other.nodeContact = otherContactIdx2;

        if(other.cable.cable)
            result.append(other);
    }

    return result;
}

void ACEIButtonNode::addCircuit(ClosedCircuit *circuit)
{
    // A circuit may pass 2 times on same node
    // But we add it only once
    if(!mCircuits.contains(circuit))
    {
        const auto items = circuit->getNode(this);
        for(const ClosedCircuit::NodeItem& item : items)
        {
            if(item.fromContact == 1 || item.toContact == 1)
            {
                mPressedCircuits.append(circuit);
                break; // It's sufficient to register once
            }
        }
    }

    AbstractCircuitNode::addCircuit(circuit);
}

void ACEIButtonNode::removeCircuit(ClosedCircuit *circuit)
{
    const auto items = circuit->getNode(this);
    for(const ClosedCircuit::NodeItem& item : items)
    {
        if(item.fromContact == 1 || item.toContact == 1)
        {
            Q_ASSERT(mPressedCircuits.contains(circuit));
            mPressedCircuits.removeOne(circuit);
            break;
        }
    }

    AbstractCircuitNode::removeCircuit(circuit);
}

bool ACEIButtonNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    setFlipContact(obj.value("flip").toBool());

    return true;
}

void ACEIButtonNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["flip"] = flipContact();
}

QString ACEIButtonNode::nodeType() const
{
    return NodeType;
}

bool ACEIButtonNode::flipContact() const
{
    return mFlipContact;
}

void ACEIButtonNode::setFlipContact(bool newFlipContact)
{
    if(mFlipContact == newFlipContact)
        return;
    mFlipContact = newFlipContact;
    emit shapeChanged();
}

ACEIButtonNode::State ACEIButtonNode::state() const
{
    return mState;
}

void ACEIButtonNode::setState(State newState)
{
    if (mState == newState)
        return;

    State oldState = mState;
    mState = newState;
    emit stateChanged();

    if(mState != State::Pressed)
    {
        // If pressed all circuits are enabled
        // Else delete old circuits
        const auto circuits = mState == State::Extracted ?
                    mCircuits : // All disabled when Extracted
                    mPressedCircuits; // Only pressed circuits
        for(ClosedCircuit *circuit : circuits)
        {
            circuit->disableCircuit();
            delete circuit;
        }
    }

    if(mState != State::Extracted)
    {
        // Scan for new circuits
        QVector<NodeContact> contactsToScan;

        if(oldState != State::Pressed)
        {
            // Add pressed contacts
            contactsToScan.append(mContacts[1]);
        }

        if(oldState == State::Extracted)
        {
            // Add normal contacts
            contactsToScan.append(mContacts[0]);
        }

        if(!contactsToScan.isEmpty())
            ClosedCircuit::createCircuitsFromOtherNode(this, contactsToScan);
    }
}
