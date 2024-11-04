/**
 * src/circuits/graphs/special/aceilevergraphitem.cpp
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

#include "aceilevergraphitem.h"

//TODO: fake
#include "../../nodes/onoffswitchnode.h"

#include "../../../objects/acei_lever/model/aceileverobject.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

#include <QtMath>

ACEILeverGraphItem::ACEILeverGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{
    mLever = new ACEILeverObject(this);
    mLever->setHasSpringReturn(true);

    connect(mLever, &ACEILeverObject::angleChanged,
            this, &ACEILeverGraphItem::triggerUpdate);
    connect(mLever, &ACEILeverObject::pressedChanged,
            this, &ACEILeverGraphItem::triggerUpdate);
}

void ACEILeverGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    constexpr double circleRadius = 5;
    constexpr double leverLength = 40;

    QRectF circle;
    circle.setSize(QSizeF(circleRadius, circleRadius));
    circle.moveCenter(center);

    // Zero is vertical up, so cos/sin are swapped
    const double angleRadiants = qDegreesToRadians(mLever->angle());

    QPointF endPt(qSin(angleRadiants),
                  qCos(angleRadiants));
    endPt *= leverLength;
    endPt += center;

    QColor color = Qt::darkCyan;
    if(mLever->isPressed())
        color = Qt::blue;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);
    pen.setWidth(5);

    // Draw lever line
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(center, endPt);

    // Draw circle
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle);
}

void ACEILeverGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        mLever->setPressed(true);
        mLastMousePos = ev->pos();
    }

    AbstractNodeGraphItem::mousePressEvent(ev);
}

void ACEILeverGraphItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    if(ev->button() & Qt::LeftButton)
    {
        QPointF delta = ev->pos() - mLastMousePos;
        if(delta.manhattanLength() > 2)
        {
            // Calculate angle
            delta = ev->pos() - center;

            // Zero is vertical up, so x/y are swapped
            const double angleRadiants = qAtan2(delta.x(), delta.y());

            int newAngle = qRound(qRadiansToDegrees(angleRadiants));
            mLever->setAngle(newAngle);

            mLastMousePos = ev->pos();
        }
    }

    AbstractNodeGraphItem::mouseMoveEvent(ev);
}

void ACEILeverGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if(ev->buttons() & Qt::LeftButton)
    {
        mLever->setPressed(false);
    }

    AbstractNodeGraphItem::mouseReleaseEvent(ev);
}
