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
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double centerOffset = 22.0;
    constexpr double relaySize = 38.0;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size + morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;
    QRectF relayRect;
    relayRect.setSize(QSizeF(relaySize, relaySize));
    relayRect.moveCenter(center);

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        relayRect.moveTop(commonLine.y1());
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        relayRect.moveBottom(commonLine.y1());
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        relayRect.moveRight(commonLine.x1());
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        relayRect.moveLeft(commonLine.x1());
        break;
    default:
        break;
    }

    drawMorsetti(painter, node()->hasCircuits(),
                 "11", "12", rotate() + TileRotate::Deg0);

    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(node()->hasCircuits() ? Qt::red : Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawLine(commonLine);

    QColor color = Qt::black;
    if(node()->relais())
    {
        switch (node()->relais()->state())
        {
        case AbstractRelais::State::Up:
            color = Qt::red;
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color.setRgb(255, 140, 140); // Light red
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }
    }

    pen.setWidthF(3.0);
    pen.setColor(color);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Draw circle
    painter->drawEllipse(relayRect);

    TileRotate textRotate = TileRotate::Deg90;
    if(rotate() == TileRotate::Deg0)
        textRotate = TileRotate::Deg270;

    if(node()->relais())
        drawName(painter, node()->objectName(), textRotate);
}

void RelaisPowerGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

void RelaisPowerGraphItem::updateRelay()
{
    if(mRelay == node()->relais())
        return;

    if(mRelay)
    {
        disconnect(mRelay, &AbstractRelais::stateChanged,
                   this, &RelaisPowerGraphItem::triggerUpdate);
    }

    mRelay = node()->relais();

    if(mRelay)
    {
        connect(mRelay, &AbstractRelais::stateChanged,
                this, &RelaisPowerGraphItem::triggerUpdate);
    }

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
