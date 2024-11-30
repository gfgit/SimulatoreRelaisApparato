#ifndef BIFILARIZATORGRAPHITEM_H
#define BIFILARIZATORGRAPHITEM_H

#include "abstractnodegraphitem.h"

class BifilarizatorNode;

class BifilarizatorGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef BifilarizatorNode Node;

    explicit BifilarizatorGraphItem(BifilarizatorNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void getConnectors(std::vector<Connector>& connectors) const final;

    BifilarizatorNode *node() const;
};

#endif // BIFILARIZATORGRAPHITEM_H
