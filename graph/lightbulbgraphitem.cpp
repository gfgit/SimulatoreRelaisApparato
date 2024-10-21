#include "lightbulbgraphitem.h"

#include "../nodes/lightbulbnode.h"

#include <QPainter>

LightBulbGraphItem::LightBulbGraphItem(LightBulbNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void LightBulbGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double centerOffset = 22.0;
    constexpr double bulbSize = 38.0;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size + morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;
    QRectF bulbRect;
    bulbRect.setSize(QSizeF(bulbSize, bulbSize));
    bulbRect.moveCenter(center);

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        bulbRect.moveTop(commonLine.y1());
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        bulbRect.moveBottom(commonLine.y1());
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        bulbRect.moveRight(commonLine.x1());
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        bulbRect.moveLeft(commonLine.x1());
        break;
    default:
        break;
    }

    drawMorsetti(painter, node()->hasCircuits(),
                 "11", "12", rotate() + TileRotate::Deg0);

    // Draw contact
    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(node()->hasCircuits() ? Qt::red : Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawLine(commonLine);

    // Draw bulb circle
    pen.setWidthF(3.0);
    painter->setPen(pen);

    if(node()->hasCircuits())
        painter->setBrush(Qt::yellow);
    else
        painter->setBrush(Qt::NoBrush);

    painter->drawEllipse(bulbRect);

    // Draw a cross onto the circle
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(bulbRect.topLeft(), bulbRect.bottomRight());
    painter->drawLine(bulbRect.topRight(), bulbRect.bottomLeft());

    TileRotate textRotate = TileRotate::Deg90;
    if(rotate() == TileRotate::Deg0)
        textRotate = TileRotate::Deg270;

    drawName(painter, node()->objectName(), textRotate);
}

void LightBulbGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

LightBulbNode *LightBulbGraphItem::node() const
{
    return static_cast<LightBulbNode *>(getAbstractNode());
}

