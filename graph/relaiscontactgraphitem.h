#ifndef RELAISCONTACTGRAPHITEM_H
#define RELAISCONTACTGRAPHITEM_H


#include "abstractnodegraphitem.h"

class RelaisContactNode;

class RelaisContactGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    RelaisContactGraphItem(RelaisContactNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void getConnectors(std::vector<Connector>& connectors) const final;

    RelaisContactNode *node() const;
};

#endif // RELAISCONTACTGRAPHITEM_H
