#ifndef SIMPLENODEGRAPHITEM_H
#define SIMPLENODEGRAPHITEM_H

#include "abstractnodegraphitem.h"

class SimpleCircuitNode;

class SimpleNodeGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef SimpleCircuitNode Node;

    SimpleNodeGraphItem(SimpleCircuitNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void getConnectors(std::vector<Connector>& connectors) const final;

    SimpleCircuitNode *node() const;
};

#endif // SIMPLENODEGRAPHITEM_H
