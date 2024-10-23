#include "onoffswitchnode.h"

#include "../core/electriccircuit.h"

OnOffSwitchNode::OnOffSwitchNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 2 sides
    mContacts.append(NodeContact("11", "12"));
    mContacts.append(NodeContact("21", "22"));
}

QVector<CableItem> OnOffSwitchNode::getActiveConnections(CableItem source, bool invertDir)
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
    default:
    case ContactType::Connected:
    case ContactType::NotConnected:
        if(m_isOn)
            return {other};
        return {};
    case ContactType::Passthrough:
        return {other};
    }

    return {};
}

QString OnOffSwitchNode::nodeType() const
{
    return NodeType;
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
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }
    else
    {
        // Disable circuits
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this);
    }
}
