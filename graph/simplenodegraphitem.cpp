#include "simplenodegraphitem.h"

#include "../nodes/simplecircuitnode.h"

#include <QPainter>

SimpleNodeGraphItem::SimpleNodeGraphItem(SimpleCircuitNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &SimpleCircuitNode::circuitsChanged,
            this, &SimpleNodeGraphItem::triggerUpdate);
}

void SimpleNodeGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // We do not draw morsetti on this node

    constexpr QPointF center(TileLocation::Size / 2.0,
                             TileLocation::Size / 2.0);
    constexpr double centerOffset = 22.0;

    QLineF lines[4] =
    {
        // Center to Deg0 South
        {center.x(), center.y() + centerOffset,
         center.x(), TileLocation::Size},

        // Center to Deg90 West
        {center.x() - centerOffset, center.y(),
         0, center.y()},

        // Center to Deg180 Nord
        {center.x(), center.y() - centerOffset,
         center.x(), 0},

        // Center to Deg270 East
        {center.x() + centerOffset, center.y(),
         TileLocation::Size, center.y()}
    };

    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(node()->hasCircuits() ? Qt::red : Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    const int startIdx = toRotateInt(rotate());
    QLineF common = lines[startIdx];
    painter->drawLine(common);

    bool hasDeg90 = true;
    bool hasDeg180 = true;
    bool hasDeg270 = true;

    if(hasDeg90)
    {
        const QLineF circuit = lines[(startIdx + 1) % 4];
        painter->drawLine(circuit);
        painter->drawLine(QLineF(circuit.p1(), common.p1()));
    }

    if(hasDeg180)
    {
        const QLineF circuit = lines[(startIdx + 2) % 4];
        painter->drawLine(circuit);
        painter->drawLine(QLineF(circuit.p1(), common.p1()));
    }

    if(hasDeg270)
    {
        const QLineF circuit = lines[(startIdx + 3) % 4];
        painter->drawLine(circuit);
        painter->drawLine(QLineF(circuit.p1(), common.p1()));
    }
}

void SimpleNodeGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate()); // Common
    connectors.emplace_back(location(), rotate() + TileRotate::Deg90);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg270);
}

SimpleCircuitNode *SimpleNodeGraphItem::node() const
{
    return static_cast<SimpleCircuitNode *>(getAbstractNode());
}
