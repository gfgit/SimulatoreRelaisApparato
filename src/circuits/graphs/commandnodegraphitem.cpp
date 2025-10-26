/**
 * src/circuits/graphs/commandnodegraphitem.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "commandnodegraphitem.h"

#include "../nodes/commandnode.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/simulationobjectfactory.h"

#include "../../views/modemanager.h"

#include "../../utils/enum_desc.h"

#include <QPainter>

CommandNodeGraphItem::CommandNodeGraphItem(CommandNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void CommandNodeGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 0;
    constexpr double circleRadius = 30.0;
    constexpr double centerOffset = circleRadius;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;

    QRectF bulbRect;
    bulbRect.setSize(QSizeF(circleRadius * 2.0, circleRadius * 2.0));
    bulbRect.moveCenter(center);

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        break;
    default:
        break;
    }

    //drawMorsetti(painter, 0, rotate() + TileRotate::Deg0);

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    // Draw common contact (0)
    bool shouldDraw = true;
    pen.setColor(getContactColor(0, &shouldDraw));
    painter->setPen(pen);
    painter->drawLine(commonLine);

    QColor color;
    switch(node()->phase())
    {
    case CommandNode::Phase::Off:
    default:
        color = Qt::darkGreen;
        break;
    case CommandNode::Phase::Waiting:
        color = Qt::darkYellow;
        break;
    case CommandNode::Phase::Retry:
        color = Qt::red;
        break;
    case CommandNode::Phase::PerformingAction:
        color = Qt::blue;
        break;
    case CommandNode::Phase::Done:
        color = Qt::cyan;
        break;
    };

    painter->fillRect(bulbRect, color);

    drawName(painter);
}

void CommandNodeGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

QString CommandNodeGraphItem::displayString() const
{
    if(node()->object())
        return node()->object()->name();
    return QLatin1String("OBJ!");
}

QString CommandNodeGraphItem::tooltipString() const
{
    if(!node()->object())
        return tr("No object set!");

    const QString objType = node()->object()->getType();
    const QString prettyType = node()->modeMgr()->objectFactory()->prettyName(objType);

    QString stateDesc;

    const int targetState = node()->targetPosition();
    EnumDesc desc;
    if(node()->getObjectPosDesc(desc))
    {
        stateDesc = desc.name(targetState);
    }

    if(stateDesc.isEmpty())
        stateDesc = tr("Unknown (%1)").arg(targetState);

    return tr("Command Node:<br>"
              "%1 <b>%2</b><br>"
              "Target State: <b>%3</b>")
            .arg(prettyType, node()->object()->name(),
                 stateDesc);
}

CommandNode *CommandNodeGraphItem::node() const
{
    return static_cast<CommandNode *>(getAbstractNode());
}
