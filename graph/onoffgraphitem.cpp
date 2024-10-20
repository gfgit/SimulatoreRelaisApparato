#include "onoffgraphitem.h"

#include "../nodes/onoffswitchnode.h"
#include "circuitscene.h"

#include <QPainter>

OnOffGraphItem::OnOffGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &OnOffSwitchNode::isOnChanged,
            this, &OnOffGraphItem::triggerUpdate);
}

void OnOffGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QLineF contactLine;
    QLineF switchLine;

    double endOffset = node()->isOn() ? 0.0 : 15.0;

    switch (toConnectorDirection(rotate() - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        switchLine.setP1(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize - 15));
        switchLine.setP2(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize + endOffset));
        break;

    case Connector::Direction::South:
        switchLine.setP1(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize - endOffset));
        switchLine.setP2(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize + 15));
        break;

    case Connector::Direction::East:
        switchLine.setP1(QPointF(TileLocation::HalfSize - endOffset,
                                 TileLocation::HalfSize + 15));
        switchLine.setP2(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize + 15));
        break;

    case Connector::Direction::West:
        switchLine.setP1(QPointF(TileLocation::HalfSize - 15,
                                 TileLocation::HalfSize + 15));
        switchLine.setP2(QPointF(TileLocation::HalfSize + endOffset,
                                 TileLocation::HalfSize + 15));
        break;
    default:
        break;
    }

    switch (toConnectorDirection(rotate() - TileRotate::Deg90))
    {
    case Connector::Direction::North:
    case Connector::Direction::South:
        contactLine.setP1(QPointF(22.0,
                                  TileLocation::HalfSize));
        contactLine.setP2(QPointF(TileLocation::Size - 22.0,
                                  TileLocation::HalfSize));
        break;

    case Connector::Direction::East:
    case Connector::Direction::West:
        contactLine.setP1(QPointF(TileLocation::HalfSize,
                                  22.0));
        contactLine.setP2(QPointF(TileLocation::HalfSize,
                                  TileLocation::Size - 22.0));
        break;
    default:
        break;
    }

    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(node()->hasCircuits() ? Qt::red : Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawLine(contactLine);
    painter->drawLine(switchLine);

    drawMorsetti(painter, node()->hasCircuits(),  "11", "12", rotate());
    drawMorsetti(painter, node()->hasCircuits() , "22", "21", rotate() + TileRotate::Deg180);

    painter->setPen(node()->isOn() ? Qt::red : Qt::black);
    drawName(painter, node()->objectName(), rotate());
}

void OnOffGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    AbstractNodeGraphItem::mouseDoubleClickEvent(e);

    auto *s = circuitScene();
    if(s && s->mode() != CircuitScene::Mode::Simulation)
        return; // TODO: block the node instead

    // Toggle on double click
    bool val = node()->isOn();
    node()->setOn(!val);
}

void OnOffGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 1);
}

OnOffSwitchNode *OnOffGraphItem::node() const
{
    return static_cast<OnOffSwitchNode *>(getAbstractNode());
}
