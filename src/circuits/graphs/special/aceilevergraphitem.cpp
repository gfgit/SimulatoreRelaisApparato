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
#include <QJsonObject>

#include "../../../objects/lever/model/genericleverobject.h"
#include "../../../objects/lever/model/genericlevermodel.h"

#include "../../../objects/lever/acei/aceileverobject.h"

#include "../../../views/modemanager.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

#include <QtMath>

ACEILeverGraphItem::ACEILeverGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void ACEILeverGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    constexpr double circleRadius = 10;
    constexpr double leverLength = 40;

    QRectF circle;
    circle.setSize(QSizeF(circleRadius * 2,
                          circleRadius * 2));
    circle.moveCenter(center);

    // Zero is vertical up, so cos/sin are swapped
    // Also returned angle must be inverted to be clockwise
    const double angleRadiants = -qDegreesToRadians(mLever ? mLever->angle() : 0);

    QPointF endPt(qSin(angleRadiants),
                  qCos(angleRadiants));
    endPt *= -leverLength; // Negative to go upwards
    endPt += center;

    QColor color = Qt::darkCyan;
    if(!mLever || mLever->isPressed())
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

    // Draw Lever name
    painter->setBrush(Qt::NoBrush);
    pen.setColor(Qt::black);
    painter->setPen(pen);
    drawName(painter,
             mLever ? mLever->name() : tr("NULL"),
             TileRotate::Deg90); // Put at bottom
}

void ACEILeverGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(getAbstractNode()->modeMgr()->mode() != FileMode::Editing
            && mLever)
    {
        if(ev->button() == Qt::LeftButton)
        {
            ev->accept();
            mLever->setPressed(true);
            mLastMousePos = ev->pos();
            return;
        }
        else if(ev->button() == Qt::RightButton)
        {
            // Customize lever
            if(ev->modifiers() == Qt::ShiftModifier)
            {
                // Toggle spring
                mLever->setHasSpringReturn(!mLever->hasSpringReturn());
            }
            else if(ev->modifiers() == Qt::AltModifier)
            {
                // Toggle absolute min
                ACEILeverPosition minPos = ACEILeverPosition(mLever->absoluteMin());
                if(minPos == ACEILeverPosition::Left)
                    minPos = ACEILeverPosition::Normal;
                else
                    minPos = ACEILeverPosition::Left;

                mLever->setAbsoluteRange(int(minPos),
                                         mLever->absoluteMax());
            }
        }
    }

    AbstractNodeGraphItem::mousePressEvent(ev);
}

void ACEILeverGraphItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    if(getAbstractNode()->modeMgr()->mode() != FileMode::Editing
            && mLever)
    {
        if(ev->buttons() & Qt::LeftButton)
        {
            QPointF delta = ev->pos() - mLastMousePos;
            if(delta.manhattanLength() > 2)
            {
                // Calculate angle
                delta = center - ev->pos();

                // Zero is vertical up, so x/y are swapped
                // Also returned angle must be inverted to be clockwise
                const double angleRadiants = -qAtan2(delta.x(), delta.y());

                int newAngle = qRound(qRadiansToDegrees(angleRadiants));

                // Disable snap with shift
                if(ev->modifiers() == Qt::ShiftModifier)
                    mLever->setAngle(newAngle);
                else
                    mLever->setAngleTrySnap(newAngle);

                mLastMousePos = ev->pos();

                ev->accept();
                return;
            }
        }
    }

    AbstractNodeGraphItem::mouseMoveEvent(ev);
}

void ACEILeverGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if(getAbstractNode()->modeMgr()->mode() != FileMode::Editing)
    {
        // We don't care about button
        // Also sometimes there are already no buttons
        if(mLever)
            mLever->setPressed(false);
    }

    AbstractNodeGraphItem::mouseReleaseEvent(ev);
}

GenericLeverObject *ACEILeverGraphItem::lever() const
{
    return mLever;
}

void ACEILeverGraphItem::setLever(GenericLeverObject *newLever)
{
    if (mLever == newLever)
        return;

    if(mLever)
    {
        disconnect(mLever, &GenericLeverObject::destroyed,
                   this, &ACEILeverGraphItem::onLeverDestroyed);

        disconnect(mLever, &GenericLeverObject::angleChanged,
                   this, &ACEILeverGraphItem::triggerUpdate);
        disconnect(mLever, &GenericLeverObject::pressedChanged,
                   this, &ACEILeverGraphItem::triggerUpdate);
        disconnect(mLever, &GenericLeverObject::objectNameChanged,
                   this, &ACEILeverGraphItem::triggerUpdate);
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &GenericLeverObject::destroyed,
                this, &ACEILeverGraphItem::onLeverDestroyed);

        connect(mLever, &GenericLeverObject::angleChanged,
                this, &ACEILeverGraphItem::triggerUpdate);
        connect(mLever, &GenericLeverObject::pressedChanged,
                this, &ACEILeverGraphItem::triggerUpdate);
        connect(mLever, &GenericLeverObject::objectNameChanged,
                this, &ACEILeverGraphItem::triggerUpdate);
    }

    emit leverChanged(mLever);
}

bool ACEILeverGraphItem::loadFromJSON(const QJsonObject &obj)
{
    QJsonObject objCopy = obj;

    // Restore fake node type
    objCopy["type"] = Node::NodeType;
    QString leverName = obj.value("lever").toString();
    setLever(getAbstractNode()->modeMgr()->leversModel()->getLever(leverName));

    return AbstractNodeGraphItem::loadFromJSON(objCopy);
}

void ACEILeverGraphItem::saveToJSON(QJsonObject &obj) const
{
    AbstractNodeGraphItem::saveToJSON(obj);

    // Replace fake node type with ours
    obj["type"] = CustomNodeType;
    obj["lever"] = mLever ? mLever->name() : QString();
}

void ACEILeverGraphItem::onLeverDestroyed()
{
    mLever = nullptr;
    emit leverChanged(mLever);
}

QString FakeLeverNode::nodeType() const
{
    return FakeLeverNode::NodeType;
}
