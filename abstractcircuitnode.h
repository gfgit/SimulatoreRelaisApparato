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

    explicit AbstractCircuitNode(QObject *parent = nullptr);

    inline int getContactCount() const { return mContacts.size(); }

    virtual QVector<CableItem> getConnections(CableItem source) = 0;

    virtual void addCircuit(ClosedCircuit *circuit);
    virtual void removeCircuit(ClosedCircuit *circuit);

    inline const QVector<NodeContact> &getContacts() const
    {
        return mContacts;
    }

    void attachCable(CableItem *item);
    void detachCable(CableItem *item);

signals:
    void circuitsChanged();

private:
    struct NodeContact
    {
        QVector<CableItem> cables;
    };

    QVector<NodeContact> mContacts;

    QVector<ClosedCircuit *> mCircuits;
};

#endif // ABSTRACTCIRCUITNODE_H
