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

#include "../../../objects/abstractsimulationobject.h"
#include "../../../objects/abstractsimulationobjectmodel.h"

#include "../../../objects/interfaces/leverinterface.h"

#include "../../../objects/lever/acei/aceileverobject.h"

#include "../../../views/modemanager.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

#include <QtMath>

ACEILeverGraphItem::ACEILeverGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{
    updateLeverTooltip();
}

void ACEILeverGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

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
    const double angleRadiants = -qDegreesToRadians(mLeverIface ? mLeverIface->angle() : 0);

    QPointF endPt(qSin(angleRadiants),
                  qCos(angleRadiants));
    endPt *= -leverLength; // Negative to go upwards
    endPt += center;

    QColor color = Qt::darkCyan;
    if(!mLeverIface || mLeverIface->isPressed())
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
            && mLeverIface)
    {
        if(ev->button() == Qt::LeftButton)
        {
            ev->accept();
            mLeverIface->setPressed(true);
            mLastMousePos = ev->pos();
            return;
        }
    }

    AbstractNodeGraphItem::mousePressEvent(ev);
}

void ACEILeverGraphItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    if(getAbstractNode()->modeMgr()->mode() != FileMode::Editing
            && mLeverIface)
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
                    mLeverIface->setAngle(newAngle);
                else
                    mLeverIface->setAngleTrySnap(newAngle);

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
        if(mLeverIface)
            mLeverIface->setPressed(false);
    }

    AbstractNodeGraphItem::mouseReleaseEvent(ev);
}

void ACEILeverGraphItem::updateLeverTooltip()
{
    if(!mLeverIface)
    {
        setToolTip(tr("NO LEVER SET!!!"));
        return;
    }

    const int leverPos = mLeverIface->position();
    const auto& desc = mLeverIface->positionDesc();

    QString posStr;
    if(mLeverIface->isPositionMiddle(leverPos))
    {
        // Position index increases going from left to right
        // so we say between left position (-1) and right position (+1)
        posStr = tr("Between<br>"
                 "<b>%1</b><br>"
                 "and<br>"
                 "<b>%2</b>")
                .arg(desc.name(leverPos - 1), desc.name(leverPos + 1));
    }
    else
    {
        posStr = tr("<b>%1</b>")
                .arg(desc.name(leverPos));
    }

    QString tipText = tr("ACEI Lever: <b>%1</b><br>"
                         "%2")
            .arg(mLever->name(), posStr);

    setToolTip(tipText);
}

AbstractSimulationObject *ACEILeverGraphItem::lever() const
{
    return mLever;
}

void ACEILeverGraphItem::setLever(AbstractSimulationObject *newLever)
{
    if(newLever && newLever->getType() != ACEILeverObject::Type)
        return;

    if(mLever)
    {
        disconnect(mLever, &AbstractSimulationObject::destroyed,
                   this, &ACEILeverGraphItem::onLeverDestroyed);
        disconnect(mLever, &AbstractSimulationObject::stateChanged,
                   this, &ACEILeverGraphItem::triggerUpdate);
        disconnect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ACEILeverGraphItem::onInterfacePropertyChanged);
        disconnect(mLever, &AbstractSimulationObject::settingsChanged,
                   this, &ACEILeverGraphItem::triggerUpdate);
        mLeverIface = nullptr;
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &AbstractSimulationObject::destroyed,
                this, &ACEILeverGraphItem::onLeverDestroyed);
        connect(mLever, &AbstractSimulationObject::stateChanged,
                this, &ACEILeverGraphItem::triggerUpdate);
        connect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ACEILeverGraphItem::onInterfacePropertyChanged);
        connect(mLever, &AbstractSimulationObject::settingsChanged,
                this, &ACEILeverGraphItem::triggerUpdate);

        mLeverIface = mLever->getInterface<LeverInterface>();
    }

    updateLeverTooltip();

    emit leverChanged(mLever);
}

bool ACEILeverGraphItem::loadFromJSON(const QJsonObject &obj)
{
    QJsonObject objCopy = obj;

    // Restore fake node type
    objCopy["type"] = Node::NodeType;

    const QString leverName = obj.value("lever").toString();
    const QString leverType = obj.value("lever_type").toString();
    auto model = getAbstractNode()->modeMgr()->modelForType(leverType);

    if(model)
        setLever(model->getObjectByName(leverName));
    else
        setLever(nullptr);

    return AbstractNodeGraphItem::loadFromJSON(objCopy);
}

void ACEILeverGraphItem::saveToJSON(QJsonObject &obj) const
{
    AbstractNodeGraphItem::saveToJSON(obj);

    // Replace fake node type with ours
    obj["type"] = CustomNodeType;

    obj["lever"] = mLever ? mLever->name() : QString();
    obj["lever_type"] = mLever ? mLever->getType() : QString();
}

void ACEILeverGraphItem::onLeverDestroyed()
{
    mLever = nullptr;
    mLeverIface = nullptr;
    updateLeverTooltip();
    emit leverChanged(mLever);
}

void ACEILeverGraphItem::onInterfacePropertyChanged(const QString &ifaceName, const QString &propName, const QVariant &value)
{
    if(ifaceName == LeverInterface::IfaceType)
    {
        if(!mLeverIface)
            return;

        if(propName == LeverInterface::PositionPropName)
        {
            updateLeverTooltip();
        }
    }
}

QString FakeLeverNode::nodeType() const
{
    return FakeLeverNode::NodeType;
}
