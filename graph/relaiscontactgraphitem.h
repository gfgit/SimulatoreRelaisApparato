#ifndef RELAISCONTACTGRAPHITEM_H
#define RELAISCONTACTGRAPHITEM_H


#include "abstractnodegraphitem.h"

class RelaisContactNode;

class RelaisContactGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    RelaisContactGraphItem(RelaisContactNode *node_);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    RelaisContactNode *node() const;
};

#endif // RELAISCONTACTGRAPHITEM_H
