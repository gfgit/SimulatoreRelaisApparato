#include "relaispowergraphitem.h"

#include "../nodes/relaispowernode.h"
#include "../abstractrelais.h"

#include <QPainter>

RelaisPowerGraphItem::RelaisPowerGraphItem(RelaisPowerNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RelaisPowerNode::relayChanged,
            this, &RelaisPowerGraphItem::updateRelay);
    updateRelay();
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
            color.setRgb(255, 140, 140); // Light red
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

void RelaisPowerGraphItem::updateRelay()
{
    if(mRelay == node()->relais())
        return;

    if(mRelay)
    {
        disconnect(mRelay, &AbstractRelais::stateChanged,
                   this, &RelaisPowerGraphItem::triggerUpdate);
        disconnect(mRelay, &AbstractRelais::objectNameChanged,
                   this, &RelaisPowerGraphItem::updateName);
    }

    mRelay = node()->relais();

    if(mRelay)
    {
        connect(mRelay, &AbstractRelais::stateChanged,
                this, &RelaisPowerGraphItem::triggerUpdate);
        connect(mRelay, &AbstractRelais::objectNameChanged,
                this, &RelaisPowerGraphItem::updateName);
    }

    updateName();
    update();
}

void RelaisPowerGraphItem::updateName()
{
    setToolTip(mRelay ?
                   mRelay->objectName() :
                   QLatin1String("NO RELAY SET"));
}

RelaisPowerNode *RelaisPowerGraphItem::node() const
{
    return static_cast<RelaisPowerNode *>(getAbstractNode());
}
