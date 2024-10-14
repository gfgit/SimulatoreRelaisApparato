#include "relaispowergraphitem.h"

#include "../nodes/relaispowernode.h"
#include "../abstractrelais.h"

#include <QPainter>

RelaisPowerGraphItem::RelaisPowerGraphItem(RelaisPowerNode *node)
    : QGraphicsObject()
    , mNode(node)
{
    setParent(mNode);

    connect(mNode, &RelaisPowerNode::relayChanged,
            this, &RelaisPowerGraphItem::updateRelay);
    updateRelay();
}

QRectF RelaisPowerGraphItem::boundingRect() const
{
    return QRectF(0, 0, 50, 50);
}

void RelaisPowerGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setWidthF(5.0);

    QColor color = Qt::black;
    if(mRelay)
    {
        switch (mRelay->state())
        {
        case AbstractRelais::State::Up:
            color = Qt::red;
            break;
        case AbstractRelais::State::Down:
            color = Qt::black;
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color.setRgb(127, 0, 0); // Light red
            break;
        default:
            break;
        }
    }

    pen.setColor(color);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Draw circle
    painter->drawEllipse(QRectF(5, 5, 45, 45));
}

void RelaisPowerGraphItem::triggerUpdate()
{
    update();
}

void RelaisPowerGraphItem::updateRelay()
{
    if(mRelay == mNode->relais())
        return;

    if(mRelay)
    {
        disconnect(mRelay, &AbstractRelais::stateChanged,
                   this, &RelaisPowerGraphItem::triggerUpdate);
    }

    mRelay = mNode->relais();

    if(mRelay)
    {
        connect(mRelay, &AbstractRelais::stateChanged,
                this, &RelaisPowerGraphItem::triggerUpdate);
    }

    update();
}
