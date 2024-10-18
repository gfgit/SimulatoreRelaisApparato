#include "onoffswitchnode.h"

#include "../closedcircuit.h"

OnOffSwitchNode::OnOffSwitchNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 2 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<AbstractCircuitNode::CableItem> OnOffSwitchNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact > 1))
        return {};

    if(!m_isOn)
        return {};

    const NodeContact& sourceContact = mContacts.at(source.nodeContact);
    if(sourceContact.getType(source.cable.pole) == ContactType::NotConnected)
        return {};

    const NodeContact& otherContact = mContacts.at(source.nodeContact == 0 ? 1 : 0);

    CableItem other;
    other.cable.cable = otherContact.cable;
    other.cable.side = otherContact.cableSide;
    other.nodeContact = source.nodeContact == 0 ? 1 : 0;
    other.cable.pole = source.cable.pole;

    switch (otherContact.getType(source.cable.pole))
    {
    case ContactType::NotConnected:
    default:
        return {};
    case ContactType::Connected:
        if(m_isOn)
            return {other};
        return {};
    case ContactType::Passthrough:
        return {other};
    }

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
