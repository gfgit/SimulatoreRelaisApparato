#include "powersourcegraphitem.h"

#include "../nodes/powersourcenode.h"

#include <QPainterPath>
#include <QPainter>

PowerSourceGraphItem::PowerSourceGraphItem(PowerSourceNode *node)
    : QGraphicsObject()
    , mNode(node)
{
    setParent(mNode);

    connect(mNode, &PowerSourceNode::enabledChanged, this, &PowerSourceGraphItem::triggerUpdate);
}

QRectF PowerSourceGraphItem::boundingRect() const
{
    return QRectF(0, 0, 50, 50);
}

void PowerSourceGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(mNode->getEnabled() ? Qt::red : Qt::darkGreen);

    // Draw a triangle
    QPointF triangle[3] = {{0, 0}, {50, 0}, {25, 50}};
    painter->drawConvexPolygon(triangle, 3);
}

void PowerSourceGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    // Toggle on double click
    bool val = mNode->getEnabled();
    mNode->setEnabled(!val);
}

void PowerSourceGraphItem::triggerUpdate()
{
    update();
}
