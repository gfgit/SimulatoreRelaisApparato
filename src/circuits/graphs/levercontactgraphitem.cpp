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

        bool unusedSpecial = false;
        auto state = node()->stateForPosition(leverNormalPos, unusedSpecial);
        Q_UNUSED(unusedSpecial);

        contactUpOn = state == LeverContactNode::State::Up;
        contactDownOn = state == LeverContactNode::State::Down;
        if(node()->swapContactState())
            std::swap(contactUpOn, contactDownOn);
    }

    drawDeviator(painter, contactUpOn, contactDownOn);

    // Draw name and lever conditions
    if(node()->lever())
    {
        // Draw lever conditions
        drawLeverConditions(painter);

        // Draw lever name
        QColor color = Qt::black;
        painter->setPen(color);
        drawName(painter);
    }
    else
    {
        // Draw lever name
        QColor color = Qt::red;
        painter->setPen(color);
        drawName(painter);
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

QRectF LeverContactGraphItem::textDisplayRect() const
{
    // Put text higher in East/West so we can draw conditions below
    const Connector::Direction arcSide = calculateArcSide();

    const QRectF conditionsRect = leverConditionsRect();

    const double textDisplayHeight = textDisplayFontSize() * 1.5;
    QRectF textRect;
    switch (textRotate())
    {
    case Connector::Direction::North:
        textRect.setTop(- TextDisplayMarginSmall - textDisplayHeight);
        textRect.setBottom(0);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);

        if(arcSide != textRotate())
            textRect.moveTop(textRect.top() + TileLocation::HalfSize / 2.0);

        if(conditionsRect.top() < TileLocation::HalfSize)
            textRect.moveBottom(qMin(textRect.bottom(), conditionsRect.top()));

        break;
    case Connector::Direction::South:
        textRect.setTop(TileLocation::Size);
        textRect.setBottom(TileLocation::Size + TextDisplayMarginSmall + textDisplayHeight);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);

        if(arcSide != textRotate())
            textRect.moveTop(textRect.top() - TileLocation::HalfSize / 2.0);

        if(conditionsRect.bottom() > TileLocation::HalfSize)
            textRect.moveTop(qMax(textRect.top(), conditionsRect.bottom()));
        break;
    case Connector::Direction::East:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::HalfSize);
        textRect.setLeft(TileLocation::Size);
        textRect.setRight(TileLocation::Size + TextDisplayMarginSmall + mTextWidth);

        if(arcSide != textRotate())
            textRect.moveLeft(textRect.left() - TileLocation::HalfSize / 2.0);

        if(deviatorNode()->hasCentralConnector())
            textRect.moveLeft(textRect.left() + 2);
        break;
    case Connector::Direction::West:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::HalfSize);
        textRect.setLeft(- TextDisplayMarginSmall - mTextWidth);
        textRect.setRight(0);

        if(arcSide != textRotate())
            textRect.moveLeft(textRect.left() + TileLocation::HalfSize / 2.0);

        if(deviatorNode()->hasCentralConnector())
            textRect.moveLeft(textRect.left() - 2);
        break;
    default:
        break;
    }

    return textRect;
}

QRectF LeverContactGraphItem::leverConditionsRect() const
{
    const double LeverConditionsWidth = 44.0;
    const double CircleMargin = 4.0;
    const Connector::Direction arcSide = calculateArcSide();

    QRectF conditionsRect;
    switch (textRotate())
    {
    case Connector::Direction::North:
    {
        conditionsRect.setTop(- LeverConditionsWidth + CircleMargin);
        conditionsRect.setBottom(CircleMargin);
        conditionsRect.setLeft(TileLocation::HalfSize - LeverConditionsWidth / 2.0);
        conditionsRect.setRight(TileLocation::HalfSize + LeverConditionsWidth / 2.0);

        if(arcSide != textRotate())
            conditionsRect.moveTop(conditionsRect.top() + TileLocation::HalfSize / 2.0);
        break;
    }
    case Connector::Direction::South:
    {
        conditionsRect.setTop(TileLocation::Size - CircleMargin);
        conditionsRect.setBottom(TileLocation::Size + LeverConditionsWidth - CircleMargin);
        conditionsRect.setLeft(TileLocation::HalfSize - LeverConditionsWidth / 2.0);
        conditionsRect.setRight(TileLocation::HalfSize + LeverConditionsWidth / 2.0);

        if(arcSide != textRotate())
            conditionsRect.moveTop(conditionsRect.top() - TileLocation::HalfSize / 2.0);
        break;
    }
    case Connector::Direction::East:
    {
        conditionsRect.setTop(TileLocation::HalfSize);
        conditionsRect.setBottom(TileLocation::Size);
        conditionsRect.setLeft(TileLocation::Size);
        conditionsRect.setRight(TileLocation::Size + LeverConditionsWidth);

        if(arcSide != textRotate())
            conditionsRect.moveLeft(conditionsRect.left() - TileLocation::HalfSize / 2.0);
        break;
    }
    case Connector::Direction::West:
    {
        conditionsRect.setTop(TileLocation::HalfSize);
        conditionsRect.setBottom(TileLocation::Size);
        conditionsRect.setLeft(- LeverConditionsWidth);
        conditionsRect.setRight(0);

        if(arcSide != textRotate())
            conditionsRect.moveLeft(conditionsRect.left() + TileLocation::HalfSize / 2.0);
        break;
    }
    default:
        break;
    }

    return conditionsRect;
}

LeverContactNode *LeverContactGraphItem::node() const
{
    return static_cast<LeverContactNode *>(getAbstractNode());
}

inline int getNextPrevPos(const int curPos, const LeverInterface *leverIface, bool next)
{
    const auto& leverPosDesc = leverIface->positionDesc();
    if(next)
    {
        for(int endPos = curPos + 1; endPos <= leverPosDesc.maxValue; endPos++)
        {
            if(!leverIface->isPositionMiddle(endPos))
                return endPos;
        }

        if(leverIface->canWarpAroundZero())
        {
            if(!leverIface->isPositionMiddle(leverPosDesc.minValue))
                return leverPosDesc.minValue;
        }
    }
    else
    {
        for(int startPos = curPos - 1; startPos >= leverPosDesc.minValue; startPos--)
        {
            if(!leverIface->isPositionMiddle(startPos))
                return startPos;
        }

        if(leverIface->canWarpAroundZero())
        {
            if(!leverIface->isPositionMiddle(leverPosDesc.maxValue))
                return leverPosDesc.maxValue;
        }
    }

    // No other position found
    return curPos;
}

QRectF LeverContactGraphItem::boundingRect() const
{
    const double extraMargin = TileLocation::HalfSize;
    QRectF base(-extraMargin, -extraMargin,
                TileLocation::Size + 2 * extraMargin, TileLocation::Size + 2 * extraMargin);

    if(mTextWidth == 0)
        return base;

    // Override to take extra space for relay arrow
    // We cannot override textDisplayRect() because otherwise text is centered
    // also on arrow space.
    const QRectF textRect = textDisplayRect();
    return base.united(textRect)
        .united(leverConditionsRect());
}

void LeverContactGraphItem::drawLeverConditions(QPainter *painter)
{
    // Draw lever conditions
    // Positioning is similar to text but opposite side
    QRectF conditionsRect = leverConditionsRect();

    const QPointF leverCenter = conditionsRect.center();

    constexpr double circleRadius = 6;
    constexpr double leverLength = 21;

    QRectF circle;
    circle.setSize(QSizeF(circleRadius * 2,
                          circleRadius * 2));
    circle.moveCenter(leverCenter);

    QPen pen;
    pen.setCapStyle(Qt::SquareCap);
    pen.setColor(Qt::black);
    pen.setWidthF(3);

    // Draw condition lines
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QPointF endPt;
    const auto conditions = node()->conditionSet();

    const LeverInterface *leverIface = node()->leverIface();

    QRectF arcRect(QPointF(),
                   QSizeF(leverLength * 2, leverLength * 2));
    arcRect.moveCenter(leverCenter);

    const auto& leverPosDesc = leverIface->positionDesc();
    const double minAngle = leverIface->angleForPosition(leverPosDesc.minValue, true);
    const double maxAngle = leverIface->angleForPosition(leverPosDesc.maxValue, true);

    const double startRadiants = -qDegreesToRadians(minAngle);
    const double endRadiants = -qDegreesToRadians(maxAngle);

    for(const LeverPositionCondition& item : conditions)
    {
        const double fromAngle = leverIface->angleForPosition(item.positionFrom, true);

        // Zero is vertical up, so cos/sin are swapped
        // Also returned angle must be inverted to be clockwise
        const double fromRadiants = -qDegreesToRadians(fromAngle);

        if(item.specialContact && item.type == LeverPositionConditionType::FromTo)
        {
            // Start from min position
            endPt = QPointF(qSin(startRadiants), qCos(startRadiants));
            endPt *= -leverLength; // Negative to go upwards
            endPt += leverCenter;

            painter->drawLine(leverCenter, endPt);

            const int switchStartPos = getNextPrevPos(item.positionFrom, leverIface, false);

            if(item.positionFrom != switchStartPos)
            {
                double switchFromAngle = leverIface->angleForPosition(switchStartPos, true);
                double switchToAngle = fromAngle;

                if(switchFromAngle > switchToAngle)
                    std::swap(switchFromAngle, switchToAngle);

                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::black);

                // Draw switch sector in black. In this sector both contacts are briefly connected.
                painter->drawPie(arcRect, (90 - switchFromAngle) * 16,
                                 (switchFromAngle - switchToAngle) * 16);

                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
            }
        }
        else
        {
            // Start at From position
            endPt = QPointF(qSin(fromRadiants), qCos(fromRadiants));
            endPt *= -leverLength; // Negative to go upwards
            endPt += leverCenter;

            painter->drawLine(leverCenter, endPt);
        }

        if(item.type == LeverPositionConditionType::FromTo)
        {
            // Draw end position line and
            // Draw arc from start position to end position
            double toAngle = leverIface->angleForPosition(item.positionTo, true);
            if(toAngle < fromAngle)
                toAngle += 360;

            const double toRadiants = -qDegreesToRadians(toAngle);

            if(item.specialContact)
            {
                // End in max position
                endPt = QPointF(qSin(endRadiants), qCos(endRadiants));
                endPt *= -leverLength; // Negative to go upwards
                endPt += leverCenter;

                painter->drawLine(leverCenter, endPt);

                // drawArc wants degrees multiplied by 16
                // Counter-clockwise and starting from 3 o'clock
                // so +90 and inverted sign
                painter->drawArc(arcRect, (90 - minAngle) * 16,
                                 (minAngle - maxAngle) * 16);

                const int switchEndPos = getNextPrevPos(item.positionTo, leverIface, true);

                if(item.positionTo != switchEndPos)
                {
                    double switchFromAngle = toAngle;
                    double switchToAngle = leverIface->angleForPosition(switchEndPos, true);

                    if(switchFromAngle > switchToAngle)
                        std::swap(switchFromAngle, switchToAngle);

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(Qt::black);

                    // Draw switch sector in black. In this sector both contacts are briefly connected.
                    painter->drawPie(arcRect, (90 - switchFromAngle) * 16,
                                     (switchFromAngle - switchToAngle) * 16);

                    painter->setPen(pen);
                    painter->setBrush(Qt::NoBrush);
                }
            }
            else
            {
                // End in To position
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
