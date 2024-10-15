#include "onoffgraphitem.h"

#include "../nodes/onoffswitchnode.h"

#include <QPainterPath>
#include <QPainter>

OnOffGraphItem::OnOffGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &OnOffSwitchNode::circuitsChanged,
            this, &OnOffGraphItem::triggerUpdate);
    connect(node(), &OnOffSwitchNode::isOnChanged,
            this, &OnOffGraphItem::triggerUpdate);
}

QRectF OnOffGraphItem::boundingRect() const
{
    return QRectF(0, 0, 50, 50);
}

void OnOffGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(node()->hasCircuits() ? Qt::red : Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    if(node()->isOn())
    {
        painter->drawLine(QLineF(25, 0, 25, 50));
        painter->drawLine(QLineF(0, 25, 25, 25));
    }
    else
    {
        painter->drawLine(QLineF(25, 0, 25, 10));
        painter->drawLine(QLineF(25, 40, 25, 50));
        painter->drawLine(QLineF(0, 25, 50, 25));
    }
}

void OnOffGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    // Toggle on double click
    bool val = node()->isOn();
    node()->setOn(!val);
}

OnOffSwitchNode *OnOffGraphItem::node() const
{
    return static_cast<OnOffSwitchNode *>(getAbstractNode());
}
