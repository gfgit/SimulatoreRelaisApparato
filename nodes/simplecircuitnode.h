#ifndef SIMPLECIRCUITNODE_H
#define SIMPLECIRCUITNODE_H

#include "../abstractcircuitnode.h"

class SimpleCircuitNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit SimpleCircuitNode(QObject *parent = nullptr);

    virtual QVector<CableItem> getConnections(CableItem source) override;
};

#endif // SIMPLECIRCUITNODE_H
