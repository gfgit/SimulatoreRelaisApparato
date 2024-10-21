#include "powersourcegraphitem.h"

#include "../nodes/powersourcenode.h"
#include "circuitscene.h"

#include <QPainterPath>
#include <QPainter>

PowerSourceGraphItem::PowerSourceGraphItem(PowerSourceNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &PowerSourceNode::enabledChanged,
            this, &PowerSourceGraphItem::triggerUpdate);
}

void PowerSourceGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(node()->getEnabled() ? Qt::red : Qt::darkGreen);

    // Draw a triangle
    const QPointF *triangle = nullptr;
    constexpr QPointF north[3] = {{20, 82}, {80, 82}, {50, 22}};
    constexpr QPointF south[3] = {{80, 18}, {20, 18}, {50, 78}};
    constexpr QPointF east[3] = {{18, 80}, {18, 20}, {78, 50}};
    constexpr QPointF west[3] = {{82, 20}, {82, 80}, {22, 50}};

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        triangle = north;
        break;

    case Connector::Direction::South:
        triangle = south;
        break;

    case Connector::Direction::East:
        triangle = east;
        break;

    case Connector::Direction::West:
        triangle = west;
        break;
    default:
        break;
    }

    if(triangle)
        painter->drawConvexPolygon(triangle, 3);

    drawMorsetti(painter, node()->hasCircuits(),  "11", "12", rotate());
}

void PowerSourceGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    AbstractNodeGraphItem::mouseDoubleClickEvent(e);

    auto *s = circuitScene();
    if(s && s->mode() == CircuitScene::Mode::Simulation)
    {
        // TODO: block the node instead
        // Toggle on double click
        bool val = node()->getEnabled();
        node()->setEnabled(!val);
    }
}

void PowerSourceGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

PowerSourceNode *PowerSourceGraphItem::node() const
{
    return static_cast<PowerSourceNode *>(getAbstractNode());
}
