#include "onoffswitchnode.h"

#include "../closedcircuit.h"

OnOffSwitchNode::OnOffSwitchNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 4 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<AbstractCircuitNode::CableItem> OnOffSwitchNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact >= getContactCount()))
        return {};

    if(!m_isOn)
        return {};

    int otherContact = (source.nodeContact + 2) % getContactCount();
    if(mContacts.at(otherContact).item.cable)
        return {mContacts.at(otherContact).item};
    return {};
}

bool OnOffSwitchNode::isOn() const
{
    return m_isOn;
}

void OnOffSwitchNode::setOn(bool newOn)
{
    if (m_isOn == newOn)
        return;
    m_isOn = newOn;
    emit isOnChanged(m_isOn);

    if(m_isOn)
    {
        QVector<NodeContact> contactsToScan;
        contactsToScan.append(mContacts[0]);
        contactsToScan.append(mContacts[1]);
        ClosedCircuit::createCircuitsFromOtherNode(this, contactsToScan);
    }
    else
    {
        const auto circuits = mCircuits;
        for(ClosedCircuit *circuit : circuits)
        {
            circuit->disableCircuit();
            delete circuit;
        }
    }
}
