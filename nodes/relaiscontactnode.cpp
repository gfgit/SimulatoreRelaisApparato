#include "relaiscontactnode.h"

#include "../closedcircuit.h"
#include "../abstractrelais.h"

RelaisContactNode::RelaisContactNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 6 sides
    mContacts.append(NodeContact()); // A
    mContacts.append(NodeContact()); // B
    mContacts.append(NodeContact()); // Down A
    mContacts.append(NodeContact()); // Down B
    mContacts.append(NodeContact()); // Up   A
    mContacts.append(NodeContact()); // Up   B
}

RelaisContactNode::~RelaisContactNode()
{
    if(mRelais)
        mRelais->removeContactNode(this);
}

QVector<AbstractCircuitNode::CableItem> RelaisContactNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact >= getContactCount()))
        return {};

    int otherContactNum = -1;
    if(mState != State::Middle)
    {
        bool isDown = mState == State::Down;
        switch (source.nodeContact)
        {
        case 0:
            otherContactNum = isDown ? 2 : 4;
            break;
        case 1:
            otherContactNum = isDown ? 3 : 5;
            break;
        case 2:
            if(isDown)
                otherContactNum = 0;
            break;
        case 3:
            if(isDown)
                otherContactNum = 1;
            break;
        case 4:
            if(!isDown)
                otherContactNum = 0;
            break;
        case 5:
            if(!isDown)
                otherContactNum = 1;
            break;
        default:
            break;
        }
    }

    if(otherContactNum != -1)
    {
        if(mContacts.at(otherContactNum).item.cable)
            return {mContacts.at(otherContactNum).item};
    }

    return {};
}

AbstractRelais *RelaisContactNode::relais() const
{
    return mRelais;
}

void RelaisContactNode::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;

    if(mRelais)
        disconnect(mRelais, &AbstractRelais::stateChanged,
                   this, &RelaisContactNode::onRelaisStateChanged);

    mRelais = newRelais;

    if(mRelais)
        connect(mRelais, &AbstractRelais::stateChanged,
                this, &RelaisContactNode::onRelaisStateChanged);

    emit relayChanged();
    onRelaisStateChanged();
}

RelaisContactNode::State RelaisContactNode::state() const
{
    return mState;
}

void RelaisContactNode::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;
    emit stateChanged();

    // Delete old circuits
    const auto circuits = mCircuits;
    for(ClosedCircuit *circuit : circuits)
    {
        circuit->disableCircuit();
        delete circuit;
    }

    if(mState != State::Middle)
    {
        // Scan for new circuits
        QVector<NodeContact> contactsToScan;
        contactsToScan.append(mContacts[0]);
        contactsToScan.append(mContacts[1]);
        ClosedCircuit::createCircuitsFromOtherNode(this, contactsToScan);
    }
}

void RelaisContactNode::onRelaisStateChanged()
{
    State s = State::Middle;
    if(mRelais)
    {
        switch (mRelais->state())
        {
        case AbstractRelais::State::Down:
            s = State::Down;
            break;
        case AbstractRelais::State::Up:
            s = State::Up;
            break;
        default:
            break;
        }
    }

    setState(s);
}
