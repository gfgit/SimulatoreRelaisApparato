#ifndef ACEIBUTTONGRAPHITEM_H
#define ACEIBUTTONGRAPHITEM_H

#include "abstractnodegraphitem.h"

class ACEIButtonNode;

class ACEIButtonGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef ACEIButtonNode Node;

    ACEIButtonGraphItem(ACEIButtonNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void getConnectors(std::vector<Connector>& connectors) const final;

    ACEIButtonNode *node() const;

public slots:
    void onShapeChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) override;
};

#endif // ACEIBUTTONGRAPHITEM_H
