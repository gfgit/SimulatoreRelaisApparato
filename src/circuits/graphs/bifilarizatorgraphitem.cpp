#include "bifilarizatorgraphitem.h"

#include "../nodes/bifilarizatornode.h"

#include <QPainter>

BifilarizatorGraphItem::BifilarizatorGraphItem(BifilarizatorNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void BifilarizatorGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    // We draw morsetti only on central connector
    drawMorsetti(painter, 1, rotate());

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double centerOffset = 0;

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

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(5.0);
    pen.setCapStyle(Qt::FlatCap);

    const QColor colors[3] =
    {
        QColor(120, 210, 255), // Light blue, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    const bool invertPolarity =
            (rotate() == TileRotate::Deg180 || rotate() == TileRotate::Deg270);

    // Draw first (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(invertPolarity ? 2 : 0))]);
    painter->setPen(pen);
    painter->drawLine(lines[toRotateInt(rotate() - TileRotate::Deg90)]);

    // Draw common (1)
    pen.setColor(colors[int(node()->hasAnyCircuit(1))]);
    painter->setPen(pen);
    painter->drawLine(lines[toRotateInt(rotate())]);

    // Draw first (2)
    pen.setColor(colors[int(node()->hasAnyCircuit(invertPolarity ? 0 : 2))]);
    painter->setPen(pen);
    painter->drawLine(lines[toRotateInt(rotate() + TileRotate::Deg90)]);
}

void BifilarizatorGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    /* Do not invert unifilar polarity for case when 2 Bifilarizator nodes
     * are placed in opposite direction and connected like this:
     *              ___+__+__+___
     *              |           |
     * Uni A -----Bifi 1       Bifi 2 ------ Uni B
     *              |           |
     *              |__-__-__-__|
     *
     * Current in Uni A must have same polarity in Uni B
     * So positive must go on upper half and negative on lower half
     * So Bifi 1 is normal and Bifi 2 is reversed
     */

    const bool invertPolarity =
            (rotate() == TileRotate::Deg180 || rotate() == TileRotate::Deg270);

    connectors.emplace_back(location(), rotate() - TileRotate::Deg90,
                            invertPolarity ? 2 : 0);
    connectors.emplace_back(location(), rotate(), 1);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg90,
                            invertPolarity ? 0 : 2);
}

BifilarizatorNode *BifilarizatorGraphItem::node() const
{
    return static_cast<BifilarizatorNode *>(getAbstractNode());
}
