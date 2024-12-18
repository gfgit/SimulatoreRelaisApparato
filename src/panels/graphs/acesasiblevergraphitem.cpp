/**
 * src/panels/graphs/acesasiblevergraphitem.cpp
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

#include "acesasiblevergraphitem.h"

//TODO: fake
#include "../../nodes/onoffswitchnode.h"
#include <QJsonObject>

#include "../../../objects/abstractsimulationobject.h"
#include "../../../objects/abstractsimulationobjectmodel.h"

#include "../../../objects/interfaces/leverinterface.h"
#include "../../../objects/interfaces/sasibaceleverextrainterface.h"

#include "../../../views/modemanager.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

#include <QtMath>

ACESasibLeverGraphItem::ACESasibLeverGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{
    updateLeverTooltip();
}

void ACESasibLeverGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    QRectF hole(QPointF(), holeSize);
    hole.moveCenter(QPointF(center.x(), center.y() + holeCenterOffsetY));

    painter->fillRect(hole, Qt::darkGray);

    constexpr double circleRadius = 8;

    QRectF circle;
    circle.setSize(QSizeF(circleRadius * 2,
                          circleRadius * 2));

    // Angle is vertical (+/- 90) -> cos is null
    // We get 0 for vertical lever and we want it to be 90, so add +90
    const double angleRadiants = qDegreesToRadians(mLeverIface ? mLeverIface->angle() + 90: 0);

    // This is inverted because Y axis increases from top to bottom (goes downwards)
    const double leverY = qCos(angleRadiants) * hole.height() / 2.0;

    const QPointF leverTip(center.x(), leverY + hole.center().y());
    circle.moveCenter(leverTip);

    QColor color = Qt::darkCyan;
    if(!mLeverIface || mLeverIface->isPressed())
        color = Qt::blue;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);
    pen.setWidth(8);

    // Draw lever line
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(hole.center(), leverTip);

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

void ACESasibLeverGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(getAbstractNode()->modeMgr()->mode() == FileMode::Simulation
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

void ACESasibLeverGraphItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
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
                QRectF hole(QPointF(), holeSize);
                hole.moveCenter(QPointF(center.x(), center.y() + holeCenterOffsetY));

                // Calculate angle
                const double leverY = ev->pos().y() - hole.center().y();

                int newAngle = 0;

                // Bound qAcos argument to (0, 1) interval
                if(ev->pos().y() > hole.bottom() || qFuzzyCompare(ev->pos().y(), hole.bottom()))
                    newAngle = -90; // Below bottom
                else if(ev->pos().y() < hole.top() || qFuzzyCompare(ev->pos().y(), hole.top()))
                    newAngle = 90; // Above top
                else
                {
                    // This is inverted because Y axis increases from top to bottom (goes downwards)
                    const double angleRadiants = qAcos(2.0 * leverY / hole.height());

                    // Angle is vertical (+/- 90)-> cos is null
                    // We get 90 for vertical but we want it to be zero for lever position, so add +90
                    newAngle = qRound(qRadiansToDegrees(angleRadiants)) - 90;
                }

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

void ACESasibLeverGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
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

void ACESasibLeverGraphItem::updateLeverTooltip()
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
        // Position index increases going upwards
        // so we say between above position (+1) and below position (-1)
        posStr = tr("Between<br>"
                 "<b>%1</b><br>"
                 "and<br>"
                 "<b>%2</b>")
                .arg(desc.name(leverPos + 1), desc.name(leverPos - 1));
    }
    else
    {
        posStr = tr("<b>%1</b>")
                .arg(desc.name(leverPos));
    }

    QString tipText = tr("ACE Lever: <b>%1</b><br>"
                         "%2")
            .arg(mLever->name(), posStr);

    if(!mLever->description().isEmpty())
    {
        tipText.append("<br><br>");
        tipText.append(mLever->description());
    }

    setToolTip(tipText);
}

AbstractSimulationObject *ACESasibLeverGraphItem::lever() const
{
    return mLever;
}

void ACESasibLeverGraphItem::setLever(AbstractSimulationObject *newLever)
{
    if(newLever && !newLever->getInterface<SasibACELeverExtraInterface>())
        return;

    if(mLever)
    {
        disconnect(mLever, &AbstractSimulationObject::destroyed,
                   this, &ACESasibLeverGraphItem::onLeverDestroyed);
        disconnect(mLever, &AbstractSimulationObject::stateChanged,
                   this, &ACESasibLeverGraphItem::triggerUpdate);
        disconnect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ACESasibLeverGraphItem::onInterfacePropertyChanged);
        disconnect(mLever, &AbstractSimulationObject::settingsChanged,
                   this, &ACESasibLeverGraphItem::triggerUpdate);
        mLeverIface = nullptr;
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &AbstractSimulationObject::destroyed,
                this, &ACESasibLeverGraphItem::onLeverDestroyed);
        connect(mLever, &AbstractSimulationObject::stateChanged,
                this, &ACESasibLeverGraphItem::triggerUpdate);
        connect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ACESasibLeverGraphItem::onInterfacePropertyChanged);
        connect(mLever, &AbstractSimulationObject::settingsChanged,
                this, &ACESasibLeverGraphItem::triggerUpdate);

        mLeverIface = mLever->getInterface<LeverInterface>();
    }

    updateLeverTooltip();

    emit leverChanged(mLever);
}

bool ACESasibLeverGraphItem::loadFromJSON(const QJsonObject &obj)
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

void ACESasibLeverGraphItem::saveToJSON(QJsonObject &obj) const
{
    AbstractNodeGraphItem::saveToJSON(obj);

    // Replace fake node type with ours
    obj["type"] = CustomNodeType;

    obj["lever"] = mLever ? mLever->name() : QString();
    obj["lever_type"] = mLever ? mLever->getType() : QString();
}

void ACESasibLeverGraphItem::onLeverDestroyed()
{
    mLever = nullptr;
    mLeverIface = nullptr;
    updateLeverTooltip();
    emit leverChanged(mLever);
}

void ACESasibLeverGraphItem::onInterfacePropertyChanged(const QString &ifaceName, const QString &propName, const QVariant &value)
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

QString FakeLeverNode2::nodeType() const
{
    return FakeLeverNode2::NodeType;
}
