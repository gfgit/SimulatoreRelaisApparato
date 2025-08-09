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

#include "../circuitscene.h"

#include "../nodes/relaispowernode.h"
#include "../../objects/relais/model/abstractrelais.h"

#include "../../views/modemanager.h"

#include "circuitcolors.h"

#include <QPainter>

#include <QJsonObject>

RelaisPowerGraphItem::RelaisPowerGraphItem(RelaisPowerNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RelaisPowerNode::relayChanged,
            this, &RelaisPowerGraphItem::updateRelay);
    updateRelay();

    recalculateTextWidth();
}

QRectF RelaisPowerGraphItem::boundingRect() const
{
    const double extraMargin = TileLocation::HalfSize;
    QRectF base(-extraMargin, -extraMargin,
                TileLocation::Size + 2 * extraMargin, TileLocation::Size + 2 * extraMargin);

    // Add space for arrow on the left
    base = base.united(textDisplayRect().adjusted(-20, 0, 0, 0));

    if(!mRelay || mRelay->relaisType() != AbstractRelais::RelaisType::Combinator)
        return base; // Default bounding rect

    // Special case for combinator relais
    // Add a whole tile to the left and to the right for drawing the X shape
    // Only one will be used based on first/second relay coil
    // But we add both otherwise there is a paint glitch when changing coil side,
    // until first scene refresh.
    QRectF xRect(-TileLocation::Size, 0,
                 TileLocation::Size, TileLocation::Size);
    QRectF xRect2(TileLocation::Size, 0,
                  TileLocation::Size, TileLocation::Size);

    return base.united(xRect).united(xRect2);
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

    const bool isCombinatorRelay = mRelay && mRelay->relaisType() == AbstractRelais::RelaisType::Combinator;

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    const QColor colors[3] =
    {
        CircuitColors::Open,
        CircuitColors::Closed,
        CircuitColors::None
    };

    // Draw common contact (0)
    // pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    // painter->setPen(pen);
    // painter->drawLine(commonLine);

    // if(node()->hasSecondConnector())
    // {
    //     // Draw second contact (1)
    //     pen.setColor(colors[int(node()->hasAnyCircuit(1))]);
    //     painter->setPen(pen);
    //     painter->drawLine(secondLine);
    // }

    AbstractRelais::State relayState = AbstractRelais::State::Down;

    QColor color = colors[int(AnyCircuitType::None)]; // Black
    if(node()->relais() && node()->modeMgr()->mode() == FileMode::Simulation)
    {
        relayState = node()->relais()->state();

        // Draw relay color only during simulation
        if(isCombinatorRelay)
        {
            AbstractRelais::State targetState = AbstractRelais::State::Down;
            AbstractRelais::State oppositeState = AbstractRelais::State::Up;
            if(node()->combinatorSecondCoil())
                std::swap(targetState, oppositeState);

            if(mRelay->state() == targetState)
                color = colors[int(AnyCircuitType::Closed)]; // Red
            else if(mRelay->state() == oppositeState)
                color = colors[int(AnyCircuitType::None)]; // Black
            else
                color = colors[int(AnyCircuitType::Open)]; // Light blue
        }
        else
        {
            if(relayState == AbstractRelais::State::Up)
            {
                SignalAspectCode code = SignalAspectCode::CodeAbsent;

                bool forceUp = false;
                bool skip = false;
                switch (node()->relais()->relaisType())
                {
                case AbstractRelais::RelaisType::Encoder:
                {
                    code = node()->relais()->getExpectedCode();
                    break;
                }
                case AbstractRelais::RelaisType::CodeRepeater:
                {
                    code = node()->relais()->getDetectedCode();
                    if(code == SignalAspectCode::CodeAbsent)
                        forceUp = true;
                    break;
                }
                default:
                    skip = true;
                    break;
                }

                if(!skip)
                {
                    // Fake relay pulsing at code frequency
                    if(node()->modeMgr()->getCodePhase(code) || forceUp)
                        relayState = AbstractRelais::State::Up;
                    else
                        relayState = AbstractRelais::State::Down;
                }
            }

            switch (relayState)
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
    }

    pen.setWidthF(10.0);
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

    const QRectF lineRect = relayRect.adjusted(-pen.widthF() / 2.0, 0,
                                               +pen.widthF() / 2.0, 0);

    if(node()->relais())
    {
        switch (node()->relais()->relaisType())
        {
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

            if(node()->relais()->relaisType() == AbstractRelais::RelaisType::Polarized)
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
        case AbstractRelais::RelaisType::Combinator:
        {
            // Draw full X near to relay circle
            QPointF startPt(0.725, 0.0534857225127);
            QPointF targetPt(2.1666666666667, 0.7799462554779);

            QPen xPen = pen;
            xPen.setColor(Qt::black);

            painter->setPen(xPen);
            painter->setBrush(Qt::NoBrush);

            if(node()->combinatorSecondCoil())
            {
                startPt.setX(1 - startPt.x());
                targetPt.setX(1 - targetPt.x());
            }

            painter->drawLine(startPt * relayRect.width() + relayRect.topLeft(),
                              targetPt * relayRect.width() + relayRect.topLeft());

            startPt.ry()  = 1 - startPt.y();
            targetPt.ry() = 1 - targetPt.y();
            painter->drawLine(startPt * relayRect.width() + relayRect.topLeft(),
                              targetPt * relayRect.width() + relayRect.topLeft());

            const QChar letter = node()->combinatorSecondCoil() ? 'R' : 'N';

            QFont f;
            f.setPointSize(relayRect.height() * 0.5);
            painter->setFont(f);
            painter->drawText(relayRect, letter, QTextOption(Qt::AlignCenter));

            break;
        }
        case AbstractRelais::RelaisType::Timer:
        {
            // Draw arcs on relay circle
            painter->setBrush(Qt::NoBrush);
            painter->setPen(pen);

            painter->drawArc(relayRect.translated(-relayRadius, 0), -90 * 16, 180 * 16);
            painter->drawArc(relayRect.translated(relayRadius, 0), -90 * 16, -180 * 16);
            break;
        }
        case AbstractRelais::RelaisType::Blinker:
        case AbstractRelais::RelaisType::Encoder:
        {
            // Draw pie sector 45 degrees
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);

            // Left
            painter->drawPie(relayRect,
                             -45 * 16, 90 * 16);

            painter->drawPie(relayRect,
                             135 * 16, 90 * 16);
            break;
        }
        case AbstractRelais::RelaisType::CodeRepeater:
        {
            // Draw horizontal center line
            painter->setBrush(Qt::NoBrush);
            painter->setPen(pen);

            const double centerY = lineRect.center().y();
            painter->drawLine(QLineF(lineRect.left(),
                                     centerY,
                                     lineRect.right(),
                                     centerY));
            break;
        }
        case AbstractRelais::RelaisType::DiskRelayAC:
        {
            // Draw a cross onto the circle
            painter->setBrush(Qt::NoBrush);
            painter->setPen(pen);

            // Adjust rect so that cross lines stay inside the circle
            const double adjLine = relayRadius * 0.27;
            const QRectF circleInner = relayRect.adjusted(adjLine, adjLine,
                                                          -adjLine, -adjLine);

            painter->drawLine(circleInner.topLeft(), circleInner.bottomRight());
            painter->drawLine(circleInner.topRight(), circleInner.bottomLeft());
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

    if(!isCombinatorRelay)
    {
        // Draw lines top/bottom
        painter->drawLine(lineRect.topLeft(), lineRect.topRight());
        painter->drawLine(lineRect.bottomLeft(), lineRect.bottomRight());

        // Draw delayed up/down lines with wider pen
        pen.setWidthF(10.0);
        painter->setPen(pen);

        // Separate a bit delay line and relay circle
        const double delayLineMargin = pen.widthF() * 0.6;

        if(node()->delayUpMillis() > 0)
        {
            painter->drawLine(QLineF(lineRect.left(), lineRect.top() - delayLineMargin,
                                     lineRect.right(), lineRect.top() - delayLineMargin));
        }
        if(node()->delayDownMillis() > 0)
        {
            painter->drawLine(QLineF(lineRect.left(), lineRect.bottom() + delayLineMargin,
                                     lineRect.right(), lineRect.bottom() + delayLineMargin));
        }
    }

    // Draw name and state arrow
    drawRelayArrow(painter, int(relayState));

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    drawName(painter);
}

void RelaisPowerGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    if(mRelay && mRelay->relaisType() == AbstractRelais::RelaisType::Combinator)
    {
        TileRotate firstConnector = TileRotate::Deg90;
        if(node()->combinatorSecondCoil())
            firstConnector = TileRotate::Deg270;

        const TileRotate secondConnector = twoConnectorsRotate() + TileRotate::Deg180;

        connectors.emplace_back(location(), firstConnector, 0);
        connectors.emplace_back(location(), secondConnector, 1);
    }
    else
    {
        connectors.emplace_back(location(), rotate(), 0);
        if(node()->hasSecondConnector())
            connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 1);
    }
}

QString RelaisPowerGraphItem::displayString() const
{
    if(mRelay)
        return mRelay->name();
    return QLatin1String("REL!");
}

QString RelaisPowerGraphItem::tooltipString() const
{
    if(!mRelay)
        return tr("No Relay set!");

    return tr("Relay <b>%1</b> (Power)<br>"
              "State: <b>%2</b>")
            .arg(mRelay->name(), mRelay->getStateName());
}

QRectF RelaisPowerGraphItem::textDisplayRect() const
{
    // When East or West give some more margin to not collide
    // with relay arrow
    // We use assume TileLocation::Size / 4 is enough

    constexpr double arrowMargin = TileLocation::Size / 4.0;

    QRectF textRect;
    switch (textRotate())
    {
    case Connector::Direction::East:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::Size);
        textRect.setLeft(TileLocation::Size + arrowMargin + TextDisplayMargin);
        textRect.setRight(TileLocation::Size + arrowMargin + 2 * TextDisplayMargin + mTextWidth);
        break;
    case Connector::Direction::West:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::Size);
        textRect.setLeft(-2 * TextDisplayMargin - mTextWidth - arrowMargin);
        textRect.setRight(-TextDisplayMargin - arrowMargin);
        break;
    default:
        // Default
        textRect = AbstractNodeGraphItem::textDisplayRect();
        break;
    }

    return textRect;
}

bool RelaisPowerGraphItem::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractNodeGraphItem::loadFromJSON(obj))
        return false;

    setPreferEastArrow(obj.value("prefer_east_arrow").toBool(false));
    return true;
}

void RelaisPowerGraphItem::saveToJSON(QJsonObject &obj) const
{
    AbstractNodeGraphItem::saveToJSON(obj);

    obj["prefer_east_arrow"] = preferEastArrow();
}

void RelaisPowerGraphItem::recalculateTextPosition()
{
    // Recalculate text label position
    std::vector<Connector> conns;
    getConnectors(conns);

    Connector::Direction arrowDirection = Connector::Direction::West;
    if(mPreferEastArrow)
        arrowDirection = Connector::Direction::East;

    if(!conns.empty())
    {
        const Connector::Direction PreferredDir[4] =
        {
            Connector::Direction::South,
            Connector::Direction::North,
            Connector::Direction::East,
            Connector::Direction::West
        };

        // Try to keep current position
        Connector::Direction possibleTextDir = textRotate();

        for(int i = 0; i < 5; i++)
        {
            bool conflict = false;

            for(const Connector& c : conns)
            {
                if(c.direction == possibleTextDir)
                {
                    conflict = true;
                    break;
                }
            }

            if(!conflict && mRelay && mRelay->relaisType() == AbstractRelais::RelaisType::Combinator)
            {
                // Do not draw text on Combinator X shape
                if((possibleTextDir == Connector::Direction::East && !node()->combinatorSecondCoil()) ||
                        (possibleTextDir == Connector::Direction::West && node()->combinatorSecondCoil()))
                    conflict = true;
            }

            if(!conflict)
            {
                setTextRotate(possibleTextDir);
                break;
            }

            // Try next
            possibleTextDir = PreferredDir[i % 4];
        }

        // Also recalculate Arrow Position
        bool eastWestFound = false;
        for(const Connector& c : conns)
        {
            if(c.direction == Connector::Direction::East)
            {
                if(arrowDirection == c.direction)
                {
                    if(eastWestFound)
                    {
                        // We cannot put Arrow neither East nor West
                        // So follow text direction
                        arrowDirection = textRotate();
                        break;
                    }

                    arrowDirection = Connector::Direction::West;
                }

                eastWestFound = true;
            }
            else if(c.direction == Connector::Direction::West)
            {
                if(arrowDirection == c.direction)
                {
                    if(eastWestFound)
                    {
                        // We cannot put Arrow neither East nor West
                        // So follow text direction
                        arrowDirection = textRotate();
                        break;
                    }

                    arrowDirection = Connector::Direction::East;
                }

                eastWestFound = true;
            }
        }
    }

    setArrowDirection(arrowDirection);
}

void RelaisPowerGraphItem::updateRelay()
{
    if(mRelay == node()->relais())
        return;

    const bool hadRelay = mRelay != nullptr;

    if(mRelay)
    {
        disconnect(mRelay, &AbstractRelais::stateChanged,
                   this, &RelaisPowerGraphItem::triggerUpdate);
        disconnect(mRelay, &AbstractRelais::typeChanged,
                   this, &RelaisPowerGraphItem::onRelayTypeChanged);
    }

    prepareGeometryChange();
    mRelay = node()->relais();
    recalculateTextWidth();

    if(mRelay)
    {
        connect(mRelay, &AbstractRelais::stateChanged,
                this, &RelaisPowerGraphItem::triggerUpdate);
        connect(mRelay, &AbstractRelais::typeChanged,
                this, &RelaisPowerGraphItem::onRelayTypeChanged);

        if(!hadRelay)
        {
            // For new relais, set default arrow position based on type
            if(mRelay->relaisType() == AbstractRelais::RelaisType::Polarized)
                setPreferEastArrow(true);
            else
                setPreferEastArrow(false);
        }
    }

    onRelayTypeChanged();
}

void RelaisPowerGraphItem::onRelayTypeChanged()
{
    prepareGeometryChange();
    recalculateTextPosition();
    update();
}

void RelaisPowerGraphItem::updateName()
{
    setToolTip(mRelay ?
                   mRelay->objectName() :
                   QLatin1String("NO RELAY SET"));
    update();
}

QRectF RelaisPowerGraphItem::arrowDisplayRect() const
{
    QRectF arrowRect;

    switch (mArrowDirection)
    {
    case Connector::Direction::North:
    case Connector::Direction::South:
    {
        QRectF textRect = textDisplayRect();
        arrowRect = textRect;
        arrowRect.setLeft(textRect.left() - 20);
        arrowRect.setRight(textRect.left() - 5);
        break;
    }
    case Connector::Direction::East:
        arrowRect.setLeft(TileLocation::Size + 5);
        arrowRect.setRight(arrowRect.left() + TileLocation::HalfSize / 2);
        arrowRect.setTop(10);
        arrowRect.setBottom(TileLocation::Size - 10);
        break;

    case Connector::Direction::West:
        arrowRect.setLeft(- TileLocation::HalfSize / 2);
        arrowRect.setRight(-5);
        arrowRect.setTop(10);
        arrowRect.setBottom(TileLocation::Size - 10);
        break;
    default:
        break;
    }

    return arrowRect;
}

void RelaisPowerGraphItem::drawRelayArrow(QPainter *painter, int relState)
{
    if(!node()->relais())
        return;

    if(mRelay->relaisType() == AbstractRelais::RelaisType::Combinator)
        return; // Combinator relais don't need arrow

    AbstractRelais::State relayState = AbstractRelais::State(relState);

    if(relayState == AbstractRelais::State::GoingUp
            || relayState == AbstractRelais::State::GoingDown)
        return; // Do not draw arrow for transitory states

    // Draw arrow up/down for normally up/down relays
    QRectF arrowRect = arrowDisplayRect();

    QLineF line;

    const double centerX = arrowRect.center().x();
    const double lineHeight = arrowRect.height() * 0.6;

    const double triangleSemiWidth = 0.5 * qMin(arrowRect.width(),
                                                arrowRect.height() - lineHeight);

    bool isRelayUp = relayState == AbstractRelais::State::Up;

    const bool isNotSimulating = node()->modeMgr()->mode() != FileMode::Simulation;
    bool isEncoderOrRepeater = (mRelay->relaisType() == AbstractRelais::RelaisType::Encoder
                                || mRelay->relaisType() == AbstractRelais::RelaisType::CodeRepeater);

    if(isNotSimulating)
    {
        // In static or editing mode,
        // draw relay in its normal state
        isRelayUp = node()->relais()->normallyUp();

        if(isEncoderOrRepeater)
            isRelayUp = false;
    }

    const QPointF arrowUpTriangle[3] =
    {
        QPointF(centerX, arrowRect.top()),
        QPointF(centerX + triangleSemiWidth, arrowRect.bottom() - lineHeight),
        QPointF(centerX - triangleSemiWidth, arrowRect.bottom() - lineHeight)
    };

    const QPointF arrowDownTriangle[3] =
    {
        QPointF(centerX, arrowRect.bottom()),
        QPointF(centerX + triangleSemiWidth, arrowRect.top() + lineHeight),
        QPointF(centerX - triangleSemiWidth, arrowRect.top() + lineHeight)
    };

    if(isRelayUp)
    {
        // Arrow up
        line.setP1(QPointF(centerX, arrowRect.bottom() - lineHeight));
        line.setP2(QPointF(centerX, arrowRect.bottom()));
    }
    else
    {
        // Arrow down
        line.setP1(QPointF(centerX, arrowRect.top() + lineHeight));
        line.setP2(QPointF(centerX, arrowRect.top()));

        if(isEncoderOrRepeater && isNotSimulating)
        {
            line.setP2(QPointF(centerX, arrowRect.bottom() - lineHeight));
        }
    }

    /* Colors:
     * Black: relay is normally down and currently down
     * Red: relay is normally up and currently up
     *
     * Blue: relay is normally up BUT currently down
     * Purple: relay is normally down BUT currently up
     */

    QColor color = Qt::black;
    if(isEncoderOrRepeater && isNotSimulating)
    {
        color = Qt::blue;
    }
    else if(isRelayUp)
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
    pen.setWidthF(5.0);
    pen.setColor(color);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(line);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(isRelayUp ? arrowUpTriangle : arrowDownTriangle, 3);

    if(isEncoderOrRepeater && isNotSimulating)
    {
        // Draw also other arrow
        painter->drawPolygon(!isRelayUp ? arrowUpTriangle : arrowDownTriangle, 3);
    }
}

bool RelaisPowerGraphItem::preferEastArrow() const
{
    return mPreferEastArrow;
}

void RelaisPowerGraphItem::setPreferEastArrow(bool newPreferEastArrow)
{
    if(mPreferEastArrow == newPreferEastArrow)
        return;

    mPreferEastArrow = newPreferEastArrow;
    recalculateTextPosition();

    CircuitScene *s = circuitScene();
    if(s)
        s->setHasUnsavedChanges(true);
}

void RelaisPowerGraphItem::setArrowDirection(Connector::Direction newArrowDirection)
{
    if(mArrowDirection == newArrowDirection)
        return;

    prepareGeometryChange();
    mArrowDirection = newArrowDirection;
    update();
}

RelaisPowerNode *RelaisPowerGraphItem::node() const
{
    return static_cast<RelaisPowerNode *>(getAbstractNode());
}
