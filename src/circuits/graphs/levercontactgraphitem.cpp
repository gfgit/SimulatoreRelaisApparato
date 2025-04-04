/**
 * src/circuits/graphs/levercontactgraphitem.cpp
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

#include "levercontactgraphitem.h"

#include "../nodes/levercontactnode.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/interfaces/leverinterface.h"

#include "../../views/modemanager.h"

#include <QPainter>

LeverContactGraphItem::LeverContactGraphItem(LeverContactNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

}

void LeverContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    const LeverInterface *leverIface = node()->leverIface();

    bool contactUpOn = node()->isContactOn(AbstractDeviatorNode::UpIdx);
    bool contactDownOn = node()->isContactOn(AbstractDeviatorNode::DownIdx);

    if(node()->lever() &&
            node()->modeMgr()->mode() != FileMode::Simulation)
    {
        // In static or editing mode,
        // draw contact with lever in its normal state
        const int leverNormalPos = leverIface->normalPosition();

        auto state = node()->stateForPosition(leverNormalPos);

        contactUpOn = state == LeverContactNode::State::Up;
        contactDownOn = state == LeverContactNode::State::Down;
        if(node()->swapContactState())
            std::swap(contactUpOn, contactDownOn);
    }

    drawDeviator(painter, contactUpOn, contactDownOn);

    TileRotate nameRotate = rotate();
    if(node()->flipContact())
        nameRotate += TileRotate::Deg180;

    // Draw name and lever conditions
    if(node()->lever())
    {
        // Draw lever conditions
        drawLeverConditions(painter, nameRotate);

        // Draw lever name
        QColor color = Qt::black;
        painter->setPen(color);
        drawName(painter, node()->lever()->name(), nameRotate);
    }
    else
    {
        // Draw lever name
        QColor color = Qt::red;
        painter->setPen(color);
        drawName(painter, tr("Null"), nameRotate);
    }
}

QString LeverContactGraphItem::displayString() const
{
    if(node()->lever())
        return node()->lever()->name();
    return QLatin1String("LEV!");
}

QString LeverContactGraphItem::tooltipString() const
{
    if(!node()->lever())
        return tr("No Lever set!");

    QString leverState;
    auto leverIface = node()->leverIface();
    if(leverIface)
    {
        const QString leverPos = leverIface->positionDesc().name(leverIface->position());
        leverState = tr("Position: <b>%1</b><br>").arg(leverPos);
    }

    return tr("Contact of lever <b>%1</b><br>"
              "%2"
              "%3")
            .arg(node()->lever()->name(),
                 leverState,
                 getContactTooltip());
}

LeverContactNode *LeverContactGraphItem::node() const
{
    return static_cast<LeverContactNode *>(getAbstractNode());
}

void LeverContactGraphItem::drawLeverConditions(QPainter *painter, TileRotate r)
{
    // Draw lever conditions
    // Positioning is similar to text but opposite side
    QRectF conditionsRect;

    switch (toConnectorDirection(r - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        // We go south, right/left (flipped)
        if(node()->flipContact())
        {
            conditionsRect.setLeft(5.0);
            conditionsRect.setRight(TileLocation::HalfSize - 22);
        }
        else
        {
            conditionsRect.setLeft(TileLocation::HalfSize + 22);
            conditionsRect.setRight(TileLocation::Size - 5.0);
        }
        conditionsRect.setTop(TileLocation::HalfSize + 10.0);
        conditionsRect.setBottom(TileLocation::Size - 15.0);
        break;

    case Connector::Direction::South:
        // We go north left/right (flipped)
        if(node()->flipContact())
        {
            conditionsRect.setLeft(TileLocation::HalfSize + 22);
            conditionsRect.setRight(TileLocation::Size - 5.0);
        }
        else
        {
            conditionsRect.setLeft(5.0);
            conditionsRect.setRight(TileLocation::HalfSize - 22);
        }
        conditionsRect.setBottom(TileLocation::HalfSize - 10.0);
        conditionsRect.setTop(15.0);
        break;

    case Connector::Direction::East:
        conditionsRect.setLeft(TileLocation::HalfSize + 3.0);
        conditionsRect.setRight(TileLocation::Size - 5.0);
        conditionsRect.setTop(TileLocation::HalfSize);
        conditionsRect.setBottom(TileLocation::Size - 23.0);
        break;

    case Connector::Direction::West:
        conditionsRect.setLeft(3.0);
        conditionsRect.setRight(TileLocation::HalfSize - 5.0);
        conditionsRect.setTop(TileLocation::HalfSize);
        conditionsRect.setBottom(TileLocation::Size - 23.0);
        break;
    default:
        break;
    }

    const QPointF leverCenter = conditionsRect.center();

    constexpr double circleRadius = 4;
    constexpr double leverLength = 12;

    QRectF circle;
    circle.setSize(QSizeF(circleRadius * 2,
                          circleRadius * 2));
    circle.moveCenter(leverCenter);

    QPen pen;
    pen.setCapStyle(Qt::SquareCap);
    pen.setColor(Qt::black);
    pen.setWidthF(2.5);

    // Draw condition lines
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QPointF endPt;
    const auto conditions = node()->conditionSet();

    const LeverInterface *leverIface = node()->leverIface();

    QRectF arcRect(QPointF(),
                   QSizeF(leverLength * 2, leverLength * 2));
    arcRect.moveCenter(leverCenter);

    for(const LeverPositionCondition& item : conditions)
    {
        const double fromAngle = leverIface->angleForPosition(item.positionFrom, true);

        // Zero is vertical up, so cos/sin are swapped
        // Also returned angle must be inverted to be clockwise
        const double fromRadiants = -qDegreesToRadians(fromAngle);

        endPt = QPointF(qSin(fromRadiants), qCos(fromRadiants));
        endPt *= -leverLength; // Negative to go upwards
        endPt += leverCenter;

        painter->drawLine(leverCenter, endPt);

        if(item.type == LeverPositionConditionType::FromTo)
        {
            // Draw end position line and
            // Draw arc from start position to end position
            double toAngle = leverIface->angleForPosition(item.positionTo, true);
            if(toAngle < fromAngle)
                toAngle += 360;

            const double toRadiants = -qDegreesToRadians(toAngle);

            endPt = QPointF(qSin(toRadiants), qCos(toRadiants));
            endPt *= -leverLength; // Negative to go upwards
            endPt += leverCenter;

            painter->drawLine(leverCenter, endPt);

            // drawArc wants degrees multiplied by 16
            // Counter-clockwise and starting from 3 o'clock
            // so +90 and inverted sign
            painter->drawArc(arcRect, (90 - fromAngle) * 16,
                             (fromAngle - toAngle) * 16);
        }
    }

    QColor leverColor = Qt::darkCyan;
    if(node()->state() == LeverContactNode::Down)
        leverColor = Qt::red;
    pen.setColor(leverColor);

    const bool drawState = node()->modeMgr()->mode() == FileMode::Simulation;
    if(drawState)
    {
        // Draw current lever state only in Simulation

        // Draw lever line
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        double leverAngle = 0;

        const int leverPos = leverIface->position();
        if(leverIface->isPositionMiddle(leverPos))
        {
            // Average prev/next angles
            const double prev = leverIface->angleForPosition(leverPos - 1, true);

            double next = 0;
            if(leverIface->canWarpAroundZero() && leverPos == leverIface->positionDesc().maxValue)
                next = leverIface->angleForPosition(0, true);
            else
                next = leverIface->angleForPosition(leverPos + 1, true);

            if(next < prev)
                next += 360;

            leverAngle = (prev + next) / 2.0;
        }
        else
        {
            leverAngle = leverIface->angleForPosition(leverPos, true);
        }

        const double leverAngleRadiants = -qDegreesToRadians(leverAngle);

        endPt = QPointF(qSin(leverAngleRadiants), qCos(leverAngleRadiants));
        endPt *= -leverLength; // Negative to go upwards
        endPt += leverCenter;
        painter->drawLine(leverCenter, endPt);
    }

    // Draw circle
    painter->setBrush(drawState ? leverColor : Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle);
}
