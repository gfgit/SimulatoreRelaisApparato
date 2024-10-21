#ifndef LIGHTBULBGRAPHITEM_H
#define LIGHTBULBGRAPHITEM_H

#include "abstractnodegraphitem.h"

class LightBulbNode;

class LightBulbGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef LightBulbNode Node;

    LightBulbGraphItem(LightBulbNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void getConnectors(std::vector<Connector>& connectors) const final;

    LightBulbNode *node() const;
};

#endif // LIGHTBULBGRAPHITEM_H
