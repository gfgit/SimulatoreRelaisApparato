#ifndef RELAISPOWERNODE_H
#define RELAISPOWERNODE_H

#include "../abstractcircuitnode.h"

class AbstractRelais;

class RelaisPowerNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit RelaisPowerNode(QObject *parent = nullptr);

    virtual QVector<CableItem> getConnections(CableItem source, bool invertDir = false) override;

    virtual void addCircuit(ClosedCircuit *circuit) override;
    virtual void removeCircuit(ClosedCircuit *circuit) override;

private:
    friend class AbstractRelais;
    AbstractRelais *mRelais;
};

#endif // RELAISPOWERNODE_H
