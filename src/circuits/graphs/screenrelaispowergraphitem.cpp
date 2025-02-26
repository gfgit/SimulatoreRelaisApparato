/**
 * src/circuits/graphs/screenrelaispowergraphitem.cpp
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

#include "screenrelaispowergraphitem.h"

#include "../nodes/screenrelaispowernode.h"
#include "../../objects/screen_relais/model/screenrelais.h"

#include "../../views/modemanager.h"

#include <QPainter>

ScreenRelaisPowerGraphItem::ScreenRelaisPowerGraphItem(ScreenRelaisPowerNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &ScreenRelaisPowerNode::relayChanged,
            this, &ScreenRelaisPowerGraphItem::updateRelay);
    updateRelay();
}

void ScreenRelaisPowerGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double centerOffset = centralCircleRadius;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;

    QRectF centralCircleRect;
    centralCircleRect.setSize(QSizeF(centralCircleRadius * 2.0, centralCircleRadius * 2.0));
    centralCircleRect.moveCenter(center);

    QRectF screenCircleRect;
    screenCircleRect.setSize(QSizeF(screenRadius * 2.0, screenRadius * 2.0));
    screenCircleRect.moveCenter(QPointF()); // We translate painter origin to center

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

    drawMorsetti(painter, 0, rotate());

    // Now draw wire
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    const QColor colors[3] =
    {
        QColor(120, 210, 255), // Light blue, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    // Draw common contact (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    painter->drawLine(commonLine);

    // Draw screen
    const double angle = mScreenRelay ? mScreenRelay->getPosition() * 38 : 0;

    painter->save();
    painter->translate(center);
    painter->rotate(angle);

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::black);
    painter->drawPie(screenCircleRect, (30) * 16, 120 * 16);

    const Qt::GlobalColor GlassColorArr[] =
    {
        Qt::black,
        Qt::red,
        Qt::yellow,
        Qt::darkGreen
    };

    painter->setBrush(GlassColorArr[mScreenRelay ? int(mScreenRelay->getColorAt(0)) : 0]);
    painter->drawEllipse(QPointF(-20, -glassOffset + 8), glassCircleRadius, glassCircleRadius);

    painter->setBrush(GlassColorArr[mScreenRelay ? int(mScreenRelay->getColorAt(1)) : 0]);
    painter->drawEllipse(QPointF(0, -glassOffset), glassCircleRadius, glassCircleRadius);

    painter->setBrush(GlassColorArr[mScreenRelay ? int(mScreenRelay->getColorAt(2)) : 0]);
    painter->drawEllipse(QPointF(+20, -glassOffset + 8), glassCircleRadius, glassCircleRadius);

    painter->restore();

    // Draw central circle
    painter->setBrush(Qt::lightGray);
    painter->setPen(Qt::black);
    painter->drawEllipse(centralCircleRect);


    // Draw name
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // TileRotate textRotate = TileRotate::Deg90;
    // drawName(painter,
    //          node()->screenRelais() ? node()->screenRelais()->name() : tr("SCREEN!"),
    //          textRotate);

    drawName(painter);
}

void ScreenRelaisPowerGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

QString ScreenRelaisPowerGraphItem::displayString() const
{
    if(node()->screenRelais())
        return node()->screenRelais()->name();
    return QLatin1String("SCR!");
}

QString ScreenRelaisPowerGraphItem::tooltipString() const
{
    if(!node()->screenRelais())
        return tr("No Screen Relay set!");

    return tr("Screen Relay <b>%1</b> (Power)<br>")
            .arg(node()->screenRelais()->name());
}

void ScreenRelaisPowerGraphItem::updateRelay()
{
    if(mScreenRelay == node()->screenRelais())
        return;

    if(mScreenRelay)
    {
        disconnect(mScreenRelay, &ScreenRelais::stateChanged,
                   this, &ScreenRelaisPowerGraphItem::triggerUpdate);
        disconnect(mScreenRelay, &ScreenRelais::settingsChanged,
                   this, &ScreenRelaisPowerGraphItem::triggerUpdate);
    }

    mScreenRelay = node()->screenRelais();

    if(mScreenRelay)
    {
        connect(mScreenRelay, &ScreenRelais::stateChanged,
                this, &ScreenRelaisPowerGraphItem::triggerUpdate);
        connect(mScreenRelay, &ScreenRelais::settingsChanged,
                this, &ScreenRelaisPowerGraphItem::triggerUpdate);
    }

    update();
}

void ScreenRelaisPowerGraphItem::updateName()
{
    setToolTip(mScreenRelay ?
                   mScreenRelay->objectName() :
                   QLatin1String("NO RELAY SET"));
    update();
}

ScreenRelaisPowerNode *ScreenRelaisPowerGraphItem::node() const
{
    return static_cast<ScreenRelaisPowerNode *>(getAbstractNode());
}
