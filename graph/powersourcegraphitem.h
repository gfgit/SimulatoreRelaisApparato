#ifndef POWERSOURCEGRAPHITEM_H
#define POWERSOURCEGRAPHITEM_H


#include "abstractnodegraphitem.h"

class PowerSourceNode;

class PowerSourceGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    PowerSourceGraphItem(PowerSourceNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

    void getConnectors(std::vector<Connector>& connectors) const final;

    PowerSourceNode *node() const;
};

#endif // POWERSOURCEGRAPHITEM_H
