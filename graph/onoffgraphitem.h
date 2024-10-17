#ifndef ONOFFGRAPHITEM_H
#define ONOFFGRAPHITEM_H


#include "abstractnodegraphitem.h"

class OnOffSwitchNode;

class OnOffGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef OnOffSwitchNode Node;

    OnOffGraphItem(OnOffSwitchNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

    void getConnectors(std::vector<Connector>& connectors) const final;

    OnOffSwitchNode *node() const;
};

#endif // ONOFFGRAPHITEM_H
