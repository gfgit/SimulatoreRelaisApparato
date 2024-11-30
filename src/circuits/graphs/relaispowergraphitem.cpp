/**
 * src/circuits/graphs/relaispowergraphitem.cpp
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

#include "relaispowergraphitem.h"

#include "../nodes/relaispowernode.h"
#include "../../objects/relais/model/abstractrelais.h"

#include "../../views/modemanager.h"

#include <QPainter>

RelaisPowerGraphItem::RelaisPowerGraphItem(RelaisPowerNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RelaisPowerNode::relayChanged,
            this, &RelaisPowerGraphItem::updateRelay);
    updateRelay();
}

void RelaisPowerGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double centerOffset = relayRadius;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;
    QLineF secondLine;
    QRectF relayRect;
    relayRect.setSize(QSizeF(relayRadius * 2.0, relayRadius * 2.0));
    relayRect.moveCenter(center);

    TileRotate r = rotate();
    if(node()->hasSecondConnector())
        r = twoConnectorsRotate() + TileRotate::Deg90;

    switch (toConnectorDirection(r))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        secondLine = centerToSouth;
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        secondLine = centerToNorth;
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        secondLine = centerToWest;
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        secondLine = centerToEast;
        break;
    default:
        break;
    }

    if(node()->hasSecondConnector())
    {
        drawMorsetti(painter, 0, twoConnectorsRotate() + TileRotate::Deg90);
        drawMorsetti(painter, 1, twoConnectorsRotate() + TileRotate::Deg270);
    }
    else
    {
        drawMorsetti(painter, 0, r + TileRotate::Deg0);
    }

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

    // Draw common contact (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    painter->drawLine(commonLine);

    if(node()->hasSecondConnector())
    {
        // Draw second contact (1)
        pen.setColor(colors[int(node()->hasAnyCircuit(1))]);
        painter->setPen(pen);
        painter->drawLine(secondLine);
    }

    QColor color = colors[int(AnyCircuitType::None)]; // Black
    if(node()->relais() && node()->modeMgr()->mode() == FileMode::Simulation)
    {
        // Draw relay color only during simulation
        switch (node()->relais()->state())
        {
        case AbstractRelais::State::Up:
            color = colors[int(AnyCircuitType::Closed)]; // Red
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color = colors[int(AnyCircuitType::Open)]; // Light blue
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }
    }

    pen.setWidthF(3.0);
    pen.setColor(color);

    if(node()->modeMgr()->mode() == FileMode::Simulation
            && node()->isTimeoutActive())
    {
        // Draw up/down timeout percent by filling relais circle
        const double timeoutStatus = node()->getTimeoutPercent();

        const double topY = relayRadius * (2 * timeoutStatus - 1);
        const double angleDeg = qRadiansToDegrees(qAsin(topY / relayRadius));
        const double arcLen = -180 - 2 * angleDeg;

        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::green);
        painter->drawChord(relayRect, angleDeg * 16, arcLen * 16);
    }

    if(node()->relais())
    {
        switch (node()->relais()->relaisType())
        {
        case AbstractRelais::RelaisType::Stabilized:
        {
            // Stabilized relais have a slice
            // The slice represent the permanent magnet
            // We use it instead to show disabling coil,
            // which if active brings the relay down.
            const QColor coilColor = qRgb(255, 140, 140);
            QColor downCoilColor = Qt::black;

            painter->setPen(Qt::NoPen);

            int upCoilAngleStart = -90 * 16;
            if(toConnectorDirection(r) == Connector::Direction::West)
            {
                upCoilAngleStart += 180 * 16;
            }

            if(node()->modeMgr()->mode() == FileMode::Simulation)
            {
                // Draw it in black if off, in light red if on
                if(node()->relais()->hasActivePowerDown())
                {
                    downCoilColor = coilColor;
                }
                else if(node()->relais()->hasActivePowerUp())
                {
                    // Only if in simulation and if down coil is off
                    // Draw up coil in light red if active
                    painter->setBrush(coilColor);
                    painter->drawPie(relayRect, upCoilAngleStart, 180 * 16);
                }
            }

            // Draw slice on second connector side
            painter->setBrush(downCoilColor);
            painter->drawPie(relayRect,
                             upCoilAngleStart - 180 * 16,
                             180 * 16);
            break;
        }
        case AbstractRelais::RelaisType::Polarized:
        case AbstractRelais::RelaisType::PolarizedInverted:
        {
            // Draw a diode symbol inside relay circle
            const double halfHeight = relayRadius * 0.6;
            QPointF triangle[3] =
            {
                {TileLocation::HalfSize - halfHeight * 0.86, TileLocation::HalfSize},
                {TileLocation::HalfSize + halfHeight, TileLocation::HalfSize - halfHeight},
                {TileLocation::HalfSize + halfHeight, TileLocation::HalfSize + halfHeight}
            };

            if(node()->relais()->relaisType() == AbstractRelais::RelaisType::PolarizedInverted)
            {
                // Invert diode
                std::swap(triangle[0].rx(), triangle[1].rx());
                triangle[2].rx() = triangle[1].x();
            }

            // Diode triangle
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
            painter->drawPolygon(triangle, 3);

            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);

            // Diode vertical line
            painter->drawLine(QLineF(triangle[0].x(), triangle[1].y(),
                                     triangle[0].x(), triangle[2].y()));
            break;
        }
        default:
            break;
        }
    }


    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Draw circle
    painter->drawEllipse(relayRect);

    // Draw lines top/bottom
    painter->drawLine(relayRect.topLeft(), relayRect.topRight());
    painter->drawLine(relayRect.bottomLeft(), relayRect.bottomRight());

    // Draw delayed up/down lines with wider pen
    pen.setWidthF(5.0);
    painter->setPen(pen);

    // Separate a bit delay line and relay circle
    const double delayLineMargin = pen.widthF() * 0.3;

    if(node()->delayUpSeconds() > 0)
    {
        painter->drawLine(QLineF(relayRect.left(), relayRect.top() - delayLineMargin,
                                 relayRect.right(), relayRect.top() - delayLineMargin));
    }
    if(node()->delayDownSeconds() > 0)
    {
        painter->drawLine(QLineF(relayRect.left(), relayRect.bottom() + delayLineMargin,
                                 relayRect.right(), relayRect.bottom() + delayLineMargin));
    }

    // Draw name and state arrow
    TileRotate textRotate = TileRotate::Deg90;
    TileRotate arrowRotate = TileRotate::Deg90;
    if(node()->hasSecondConnector())
    {
        textRotate = twoConnectorsRotate() + TileRotate::Deg90;
        arrowRotate = textRotate + TileRotate::Deg90;
    }
    else
    {
        if(r == TileRotate::Deg0)
            textRotate = TileRotate::Deg270;
        if(r == TileRotate::Deg90)
            arrowRotate = TileRotate::Deg270;
    }

    drawRelayArrow(painter, arrowRotate);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    drawName(painter,
             node()->relais() ? node()->relais()->name() : tr("REL!"),
             textRotate);
}

void RelaisPowerGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    if(node()->hasSecondConnector())
    {
        // Always put connectors horizontal
        // We ignore other rotations otherwise we cannot draw name :(
        connectors.emplace_back(location(), twoConnectorsRotate() + TileRotate::Deg90, 0);
        connectors.emplace_back(location(), twoConnectorsRotate() + TileRotate::Deg270, 1);
    }
    else
    {
        connectors.emplace_back(location(), rotate(), 0);
    }
}

void RelaisPowerGraphItem::updateRelay()
{
    if(mRelay == node()->relais())
        return;

    if(mRelay)
    {
        disconnect(mRelay, &AbstractRelais::stateChanged,
                   this, &RelaisPowerGraphItem::triggerUpdate);
    }

    mRelay = node()->relais();

    if(mRelay)
    {
        connect(mRelay, &AbstractRelais::stateChanged,
                this, &RelaisPowerGraphItem::triggerUpdate);
    }

    update();
}

void RelaisPowerGraphItem::updateName()
{
    setToolTip(mRelay ?
                   mRelay->objectName() :
                   QLatin1String("NO RELAY SET"));
    update();
}

void RelaisPowerGraphItem::drawRelayArrow(QPainter *painter,
                                          TileRotate r)
{
    if(!node()->relais())
        return;

    if(node()->relais()->state() == AbstractRelais::State::GoingUp
            || node()->relais()->state() == AbstractRelais::State::GoingDown)
        return; // Do not draw arrow for transitory states

    // Draw arrow up/down for normally up/down relays
    QRectF arrowRect;

    switch (toConnectorDirection(r))
    {
    case Connector::Direction::North:
        arrowRect.setLeft(10.0);
        arrowRect.setRight(TileLocation::HalfSize);
        arrowRect.setTop(15.0);
        arrowRect.setBottom(TileLocation::HalfSize - relayRadius + 5.0);
        break;

    case Connector::Direction::South:
        arrowRect.setLeft(10.0);
        arrowRect.setRight(TileLocation::HalfSize);
        arrowRect.setTop(TileLocation::HalfSize + relayRadius - 5.0);
        arrowRect.setBottom(TileLocation::Size - 15.0);
        break;

    case Connector::Direction::East:
        arrowRect.setLeft(TileLocation::HalfSize + relayRadius + 5.0);
        arrowRect.setRight(TileLocation::Size - 5.0);
        arrowRect.setTop(TileLocation::HalfSize - relayRadius);
        arrowRect.setBottom(TileLocation::HalfSize + relayRadius);
        break;

    case Connector::Direction::West:
        arrowRect.setLeft(5.0);
        arrowRect.setRight(TileLocation::HalfSize - relayRadius - 5.0);
        arrowRect.setTop(TileLocation::HalfSize - relayRadius);
        arrowRect.setBottom(TileLocation::HalfSize + relayRadius);
        break;
    default:
        break;
    }

    QLineF line;
    QPointF triangle[3];

    const double centerX = arrowRect.center().x();
    const double lineHeight = arrowRect.height() * 0.6;

    const double triangleSemiWidth = 0.5 * qMin(arrowRect.width(),
                                                arrowRect.height() - lineHeight);

    const bool isRelayUp = node()->relais()->state() == AbstractRelais::State::Up;
    if(isRelayUp)
    {
        // Arrow up
        line.setP1(QPointF(centerX, arrowRect.bottom() - lineHeight));
        line.setP2(QPointF(centerX, arrowRect.bottom()));

        triangle[0] = QPointF(centerX, arrowRect.top());
        triangle[1] = QPointF(centerX + triangleSemiWidth, line.y1());
        triangle[2] = QPointF(centerX - triangleSemiWidth, line.y1());
    }
    else
    {
        // Arrow down
        line.setP1(QPointF(centerX, arrowRect.top() + lineHeight));
        line.setP2(QPointF(centerX, arrowRect.top()));

        triangle[0] = QPointF(centerX, arrowRect.bottom());
        triangle[1] = QPointF(centerX + triangleSemiWidth, line.y1());
        triangle[2] = QPointF(centerX - triangleSemiWidth, line.y1());
    }

    /* Colors:
     * Black: relay is normally down and currently down
     * Red: relay is normally up and currently up
     *
     * Blue: relay is normally up BUT currently down
     * Purple: relay is normally down BUT currently up
     */

    QColor color = Qt::black;
    if(isRelayUp)
    {
        if(node()->relais()->normallyUp())
            color = Qt::red; // Relay in normal state
        else
            color = Qt::darkMagenta;
    }
    else
    {
        if(!node()->relais()->normallyUp())
            color = Qt::black; // Relay in normal state
        else
            color = Qt::blue;
    }

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(3.0);
    pen.setColor(color);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(line);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(triangle, 3);
}

RelaisPowerNode *RelaisPowerGraphItem::node() const
{
    return static_cast<RelaisPowerNode *>(getAbstractNode());
}
