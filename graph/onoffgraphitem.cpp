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
    QLineF commonLine;
    QLineF contact1Line;
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

    const auto cableDirection = toConnectorDirection(rotate());
    switch (cableDirection)
    {
    case Connector::Direction::South:
    case Connector::Direction::North:
        // From switch to South
        commonLine.setP1(QPointF(TileLocation::HalfSize,
                                 TileLocation::HalfSize + 15));
        commonLine.setP2(QPointF(TileLocation::HalfSize,
                                 TileLocation::Size - 22.0));

        // From switch to North
        contact1Line.setP1(commonLine.p1());
        contact1Line.setP2(QPointF(TileLocation::HalfSize, 22.0));

        if(cableDirection == Connector::Direction::North)
            std::swap(commonLine, contact1Line);

        break;

    case Connector::Direction::East:
    case Connector::Direction::West:
        // From switch to East
        commonLine.setP1(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize));
        commonLine.setP2(QPointF(TileLocation::Size - 22.0,
                                 TileLocation::HalfSize));

        // From switch to West
        contact1Line.setP1(commonLine.p1());
        contact1Line.setP2(QPointF(22.0, TileLocation::HalfSize));

        if(cableDirection == Connector::Direction::West)
            std::swap(commonLine, contact1Line);
        break;
    default:
        break;
    }

    const QColor closedColor = Qt::red;
    const QColor openColor(255, 140, 140); // Light red

    // Draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(5.0);
    pen.setCapStyle(Qt::FlatCap);

    // Draw common contact (0)
    if(node()->hasCircuit(0, CircuitType::Closed))
        pen.setColor(closedColor);
    else if(node()->hasCircuit(0, CircuitType::Open))
        pen.setColor(openColor);
    else
        pen.setColor(Qt::black);

    painter->setPen(pen);
    painter->drawLine(commonLine);

    // Draw first contact (1)
    if(node()->hasCircuit(1, CircuitType::Closed))
        pen.setColor(closedColor);
    else if(node()->hasCircuit(1, CircuitType::Open))
        pen.setColor(openColor);
    else
        pen.setColor(Qt::black);

    painter->setPen(pen);
    painter->drawLine(contact1Line);

    // Draw switch line
    if(node()->hasCircuits(CircuitType::Closed))
        pen.setColor(closedColor);
    else if(node()->hasCircuit(0, CircuitType::Open) && node()->hasCircuit(1, CircuitType::Open))
        pen.setColor(openColor);
    else
        pen.setColor(Qt::black);

    painter->setPen(pen);
    painter->drawLine(switchLine);

    drawMorsetti(painter, 0, rotate());
    drawMorsetti(painter, 1, rotate() + TileRotate::Deg180);

    painter->setPen(node()->isOn() ? Qt::red : Qt::black);
    drawName(painter, node()->objectName(), rotate());
}

void OnOffGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    AbstractNodeGraphItem::mouseDoubleClickEvent(e);

    auto *s = circuitScene();
    if(s && s->mode() == CircuitScene::Mode::Simulation)
    {
        // TODO: block the node instead
        // Toggle on double click
        bool val = node()->isOn();
        node()->setOn(!val);
    }
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
