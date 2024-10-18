#include "relaiscontactgraphitem.h"

#include "../nodes/relaiscontactnode.h"
#include "../abstractrelais.h"

#include <QPainter>

RelaisContactGraphItem::RelaisContactGraphItem(RelaisContactNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RelaisContactNode::stateChanged,
            this, &RelaisContactGraphItem::triggerUpdate);
}

void RelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    constexpr QPointF center(TileLocation::Size / 2.0,
                             TileLocation::Size / 2.0);
    constexpr double morsettiOffset = 22.0;

    constexpr QLineF centerToNorth(center,
                               QPointF(center.x(), morsettiOffset));

    constexpr QLineF centerToSouth(center,
                               QPointF(center.x(),
                                       TileLocation::Size - morsettiOffset));

    constexpr QLineF centerToEast(center,
                              QPointF(TileLocation::Size - morsettiOffset,
                                      center.y()));

    constexpr QLineF centerToWest(center,
                              QPointF(morsettiOffset,
                                      center.y()));

    QLineF commonLine;
    QLineF contact1Line;
    QLineF contact2Line;

    bool commonOn = node()->hasCircuits();
    bool contact1On = node()->state() == RelaisContactNode::State::Up;
    bool contact2On = node()->state() == RelaisContactNode::State::Down;

    int startAngle = 0;
    int endAngle = 0;

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        contact1Line = centerToEast;
        contact2Line = centerToSouth;

        startAngle = 0;
        endAngle = -90;
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        contact1Line = centerToWest;
        contact2Line = centerToNorth;

        startAngle = -180;
        endAngle = -270;
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        contact1Line = centerToSouth;
        contact2Line = centerToWest;

        startAngle = -90;
        endAngle = -180;
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        contact1Line = centerToNorth;
        contact2Line = centerToEast;

        startAngle = 90;
        endAngle = 0;
        break;
    default:
        break;
    }

    if(!contact1On)
        startAngle += 35;
    if(!contact2On)
        endAngle -= 35;

    drawMorsetti(painter, commonOn,   "11", "12", rotate() + TileRotate::Deg0);
    drawMorsetti(painter, commonOn && contact1On, "21", "22", rotate() + TileRotate::Deg90);
    drawMorsetti(painter, commonOn && contact2On, "32", "31", rotate() + TileRotate::Deg180);

    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawLine(commonLine);
    painter->drawLine(contact1Line);
    painter->drawLine(contact2Line);

    // Draw Arc
    const QRectF arcRect(center.x() - 15,
                         center.y() - 15,
                         30, 30);

    painter->drawArc(arcRect,
                     startAngle * 16,
                     (endAngle - startAngle) * 16);

    if(commonOn)
    {
        // Now draw powered wires on top
        pen.setColor(Qt::red);
        painter->setPen(pen);

        painter->drawLine(commonLine);

        if(contact1On)
            painter->drawLine(contact1Line);

        if(contact2On)
            painter->drawLine(contact2Line);
    }

    if(node()->relais())
    {
        QColor color = Qt::black;

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

        painter->setPen(color);
        drawName(painter, node()->objectName(), rotate());
    }
}

void RelaisContactGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0); // Common
    connectors.emplace_back(location(), rotate() + TileRotate::Deg90, 1);  // Up
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 2); // Down
}

RelaisContactNode *RelaisContactGraphItem::node() const
{
    return static_cast<RelaisContactNode *>(getAbstractNode());
}
