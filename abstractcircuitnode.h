#ifndef ABSTRACTCIRCUITNODE_H
#define ABSTRACTCIRCUITNODE_H

#include <QObject>
#include <QVector>

#include "circuitcable.h"

class ClosedCircuit;

class AbstractCircuitNode : public QObject
{
    Q_OBJECT
public:
    struct CableItem
    {
        CircuitCable *cable = nullptr;
        CircuitCable::Side cableSide = CircuitCable::Side::A1;
        int nodeContact = 0;
    };

    struct NodeContact
    {
        QString name1;
        QString name2;
        CableItem item;
    };

    explicit AbstractCircuitNode(QObject *parent = nullptr);
    ~AbstractCircuitNode();

    inline int getContactCount() const { return mContacts.size(); }

    virtual QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) = 0;

    virtual void addCircuit(ClosedCircuit *circuit);
    virtual void removeCircuit(ClosedCircuit *circuit);

    inline const QVector<NodeContact> &getContacts() const
    {
        return mContacts;
    }

    inline bool hasCircuits() const { return mCircuits.size(); }

    void attachCable(CableItem item);
    void detachCable(CableItem item);

signals:
    void circuitsChanged();

protected:
    QVector<NodeContact> mContacts;

    QVector<ClosedCircuit *> mCircuits;
};

inline bool operator ==(const AbstractCircuitNode::CableItem& lhs,
                        const AbstractCircuitNode::CableItem& rhs)
{
    // NOTE: nodeContact should be redundant to check
    return lhs.cable == rhs.cable && lhs.cableSide == rhs.cableSide;
}

#endif // ABSTRACTCIRCUITNODE_H
