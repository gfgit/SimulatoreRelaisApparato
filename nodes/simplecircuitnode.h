#ifndef SIMPLECIRCUITNODE_H
#define SIMPLECIRCUITNODE_H

#include "../abstractcircuitnode.h"

class SimpleCircuitNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit SimpleCircuitNode(QObject *parent = nullptr);

    virtual QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    virtual void addCircuit(ClosedCircuit *circuit);
    virtual void removeCircuit(ClosedCircuit *circuit);

    inline int disabledContact() const
    {
        return mDisabledContact;
    }

    void setDisabledContact(int val);

    inline bool isContactEnabled(int nodeContact) const
    {
        if(nodeContact == 0)
            return true; // Common is always enables
        return nodeContact != mDisabledContact;
    }

    inline bool hasCircuit(int nodeContact) const
    {
        Q_ASSERT(nodeContact >= 0 && nodeContact < 4);
        if(nodeContact == 0)
            return hasCircuits(); // Common
        return mCircuitCount[nodeContact - 1] > 0;
    }

signals:
    void disabledContactChanged();

private:
    int mDisabledContact = 0; // All enabled
    int mCircuitCount[3] = {0, 0, 0}; // Circuits on every branch
};

#endif // SIMPLECIRCUITNODE_H
