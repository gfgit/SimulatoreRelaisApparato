#include "simplenodegraphitem.h"

#include "../nodes/simplecircuitnode.h"

#include <QPainter>

SimpleNodeGraphItem::SimpleNodeGraphItem(SimpleCircuitNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &SimpleCircuitNode::disabledContactChanged,
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
    pen.setColor(Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    const int startIdx = toRotateInt(rotate());
    QLineF common = lines[startIdx];
    painter->drawLine(common);

    bool hasDeg90  = node()->disabledContact() != 1;
    bool hasDeg180 = node()->disabledContact() != 2;
    bool hasDeg270 = node()->disabledContact() != 3;

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

    if(node()->hasCircuits())
    {
        pen.setColor(Qt::red);
        painter->setPen(pen);

        QLineF common = lines[startIdx];
        painter->drawLine(common);

        // Redraw powered wires on top
        if(hasDeg90 && node()->hasCircuit(1))
        {
            const QLineF circuit = lines[(startIdx + 1) % 4];
            painter->drawLine(circuit);
            painter->drawLine(QLineF(circuit.p1(), common.p1()));
        }

        if(hasDeg180 && node()->hasCircuit(2))
        {
            const QLineF circuit = lines[(startIdx + 2) % 4];
            painter->drawLine(circuit);
            painter->drawLine(QLineF(circuit.p1(), common.p1()));
        }

        if(hasDeg270 && node()->hasCircuit(3))
        {
            const QLineF circuit = lines[(startIdx + 3) % 4];
            painter->drawLine(circuit);
            painter->drawLine(QLineF(circuit.p1(), common.p1()));
        }
    }
}

void SimpleNodeGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    bool hasDeg90  = node()->disabledContact() != 1;
    bool hasDeg180 = node()->disabledContact() != 2;
    bool hasDeg270 = node()->disabledContact() != 3;

    connectors.emplace_back(location(), rotate(), 0); // Common

    if(hasDeg90)
        connectors.emplace_back(location(), rotate() + TileRotate::Deg90, 1);

    if(hasDeg180)
        connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 2);

    if(hasDeg270)
        connectors.emplace_back(location(), rotate() + TileRotate::Deg270, 3);
}

SimpleCircuitNode *SimpleNodeGraphItem::node() const
{
    return static_cast<SimpleCircuitNode *>(getAbstractNode());
}
