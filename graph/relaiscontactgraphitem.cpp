#include "relaiscontactgraphitem.h"

#include "../nodes/relaiscontactnode.h"

#include <QPainterPath>
#include <QPainter>

RelaisContactGraphItem::RelaisContactGraphItem(RelaisContactNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RelaisContactNode::circuitsChanged,
            this, &RelaisContactGraphItem::triggerUpdate);
    connect(node(), &RelaisContactNode::stateChanged,
            this, &RelaisContactGraphItem::triggerUpdate);
}

QRectF RelaisContactGraphItem::boundingRect() const
{
    return QRectF(0, 0, 50, 50);
}

void RelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(node()->hasCircuits() ? Qt::red : Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawLine(QLineF(25, 0, 25, 50));
    painter->drawLine(QLineF(25, 25, 50, 25));

    int startAngle = node()->state() == RelaisContactNode::State::Down ?
                0 : -40 * 16;

    int endAngle = node()->state() == RelaisContactNode::State::Up ?
                90 * 16 : 130 * 16;

    painter->drawArc(QRectF(10, 10, 30, 30),
                     startAngle,
                     endAngle - startAngle);
}

RelaisContactNode *RelaisContactGraphItem::node() const
{
    return static_cast<RelaisContactNode *>(getAbstractNode());
}
