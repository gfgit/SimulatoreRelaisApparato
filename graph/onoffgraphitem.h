#ifndef ONOFFGRAPHITEM_H
#define ONOFFGRAPHITEM_H


#include "abstractnodegraphitem.h"

class OnOffSwitchNode;

class OnOffGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    OnOffGraphItem(OnOffSwitchNode *node_);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

    OnOffSwitchNode *node() const;
};

#endif // ONOFFGRAPHITEM_H
