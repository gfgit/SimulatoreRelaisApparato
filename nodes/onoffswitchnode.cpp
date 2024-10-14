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

QVector<AbstractCircuitNode::CableItem> OnOffSwitchNode::getConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact > 3))
        return {};

    const NodeContact& contact = mContacts[source.nodeContact];

    int otherContactNum = 0;
    switch (source.nodeContact)
    {
    case 0:
        otherContactNum = 2;
        break;
    case 1:
        otherContactNum = 3;
        break;
    case 2:
        otherContactNum = 0;
        break;
    case 3:
        otherContactNum = 1;
        break;
    default:
        break;
    }

    const NodeContact& otherContact = mContacts[otherContactNum];

    int cableCount = contact.cables.size();
    if(m_isOn)
        cableCount += otherContact.cables.size();

    QVector<AbstractCircuitNode::CableItem> result;
    result.reserve(cableCount);

    for(const CableItem& item : contact.cables)
    {
        if(item == source)
            continue;

        result.append(item);
    }

    if(m_isOn)
    {
        for(const CableItem& item : otherContact.cables)
        {
            result.append(item);
        }
    }

    return result;
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
        ClosedCircuit::createCircuitsFromOtherNode(this);
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
