/**
 * src/panels/graphs/aceilevergraphitem.cpp
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

//TODO: remove BEM
#include "../../../objects/lever/bem/bemleverobject.h"

#include "../../../objects/simple_activable/lightbulbobject.h"

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

    // Zero is vertical up, so cos/sin are swapped
    // Also returned angle must be inverted to be clockwise
    const double angleRadiants = -qDegreesToRadians(mLeverIface ? mLeverIface->angle() : 0);
    const QPointF delta(qSin(angleRadiants),
                        qCos(angleRadiants));

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    constexpr QPointF leverCenter(center.x(), center.y());

    constexpr double baseCircleRadius = 32;
    constexpr double leverCircleRadius = 18;
    constexpr double leverTipLength = 32;
    constexpr double leverBottomLength = 24;

    constexpr double lightCircleRadius = 12;
    constexpr double lightOffset = 16;

    // Draw dark gray border around
    QPen borderPen;
    borderPen.setWidth(3);
    borderPen.setColor(Qt::darkGray);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());

    // Draw lights
    QRectF circle;
    circle.setSize(QSizeF(lightCircleRadius * 2,
                          lightCircleRadius * 2));

    if(mLeftLight)
    {
        // White light (We draw yellow to have contrast)
        // and because incandescent light bulb are never white
        if(mLeftLight->state() == LightBulbObject::State::On)
            painter->setBrush(Qt::yellow);
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(lightOffset, lightOffset));
        painter->drawEllipse(circle);
    }

    if(mRightLight)
    {
        // Blue light
        if(mRightLight->state() == LightBulbObject::State::On)
            painter->setBrush(Qt::blue);
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(TileLocation::Size - lightOffset, lightOffset));
        painter->drawEllipse(circle);
    }

    // Draw base circle below lever
    circle.setSize(QSizeF(baseCircleRadius * 2,
                          baseCircleRadius * 2));
    circle.moveCenter(leverCenter);
    painter->setBrush(Qt::lightGray);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle);

    // Draw lever
    QColor color = qRgb(77, 77, 77); // Dark gray
    if(!mLeverIface || mLeverIface->isPressed())
        color = Qt::blue;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);
    pen.setWidth(12);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Lever tip
    QPointF endPt = delta;
    endPt *= -leverTipLength; // Negative to go upwards
    endPt += leverCenter;
    painter->drawLine(leverCenter, endPt);

    // Lever bottom
    pen.setWidth(20);
    painter->setPen(pen);

    endPt = delta;
    endPt *= leverBottomLength; // Positive to go downwards
    endPt += leverCenter;
    painter->drawLine(leverCenter, endPt);

    // Draw circle
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);

    circle.setSize(QSizeF(leverCircleRadius * 2,
                          leverCircleRadius * 2));
    circle.moveCenter(leverCenter);
    painter->drawEllipse(circle);

    // Draw Lever name
    const QString leverName = mLever ? mLever->name() : tr("NULL");

    QRectF textRect;
    textRect.setLeft(10);
    textRect.setRight(TileLocation::Size - 10.0);
    textRect.setTop(TileLocation::HalfSize + 20.0);
    textRect.setBottom(TileLocation::Size - 4.0);

    Qt::Alignment textAlign = Qt::AlignLeft;

    QFont f;
    f.setPointSizeF(18.0);
    f.setBold(true);

    QFontMetrics metrics(f, painter->device());
    double width = metrics.horizontalAdvance(leverName, QTextOption(textAlign));
    if(width > textRect.width())
    {
        f.setPointSizeF(f.pointSizeF() * textRect.width() / width * 0.9);
    }

    painter->setBrush(Qt::NoBrush);
    pen.setColor(Qt::black);
    painter->setPen(pen);

    painter->setFont(f);
    painter->drawText(textRect, textAlign, leverName);
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
        int prevPos = leverPos - 1;
        int nextPos = leverPos + 1;

        if(mLeverIface->canWarpAroundZero())
        {
            if(leverPos == mLeverIface->positionDesc().maxValue)
                nextPos = 0; // Wrap around, next is first position
        }

        // Position index increases going from left to right
        // so we say between left position (-1) and right position (+1)
        posStr = tr("Between<br>"
                 "<b>%1</b><br>"
                 "and<br>"
                 "<b>%2</b>")
                .arg(desc.name(prevPos), desc.name(nextPos));
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

LightBulbObject *ACEILeverGraphItem::leftLight() const
{
    return mLeftLight;
}

void ACEILeverGraphItem::setLeftLight(LightBulbObject *newLeftLight)
{
    if(mLeftLight == newLeftLight)
        return;

    if(mLeftLight)
    {
        disconnect(mLeftLight, &LightBulbObject::stateChanged,
                   this, &ACEILeverGraphItem::triggerUpdate);
        disconnect(mLeftLight, &LightBulbObject::destroyed,
                   this, &ACEILeverGraphItem::onLightDestroyed);
    }

    mLeftLight = newLeftLight;

    if(mLeftLight)
    {
        connect(mLeftLight, &LightBulbObject::stateChanged,
                this, &ACEILeverGraphItem::triggerUpdate);
        connect(mLeftLight, &LightBulbObject::destroyed,
                this, &ACEILeverGraphItem::onLightDestroyed);
    }

    getAbstractNode()->modeMgr()->setFileEdited();
    update();
    emit lightsChanged();
}

LightBulbObject *ACEILeverGraphItem::rightLight() const
{
    return mRightLight;
}

void ACEILeverGraphItem::setRightLight(LightBulbObject *newRightLight)
{
    if(mRightLight == newRightLight)
        return;

    if(mRightLight)
    {
        disconnect(mRightLight, &LightBulbObject::stateChanged,
                   this, &ACEILeverGraphItem::triggerUpdate);
        disconnect(mRightLight, &LightBulbObject::destroyed,
                   this, &ACEILeverGraphItem::onLightDestroyed);
    }

    mRightLight = newRightLight;

    if(mRightLight)
    {
        connect(mRightLight, &LightBulbObject::stateChanged,
                this, &ACEILeverGraphItem::triggerUpdate);
        connect(mRightLight, &LightBulbObject::destroyed,
                this, &ACEILeverGraphItem::onLightDestroyed);
    }

    getAbstractNode()->modeMgr()->setFileEdited();
    update();
    emit lightsChanged();
}

AbstractSimulationObject *ACEILeverGraphItem::lever() const
{
    return mLever;
}

void ACEILeverGraphItem::setLever(AbstractSimulationObject *newLever)
{
    // TODO: remove BEM
    if(newLever && newLever->getType() != ACEILeverObject::Type && newLever->getType() != BEMLeverObject::Type)
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

    auto lightModel = getAbstractNode()->modeMgr()->modelForType(LightBulbObject::Type);
    if(lightModel)
    {
        const QString leftLightName = obj.value("light_left").toString();
        setLeftLight(static_cast<LightBulbObject *>(lightModel->getObjectByName(leftLightName)));

        const QString rightLightName = obj.value("light_right").toString();
        setRightLight(static_cast<LightBulbObject *>(lightModel->getObjectByName(rightLightName)));
    }
    else
    {
        setLeftLight(nullptr);
        setRightLight(nullptr);
    }

    return AbstractNodeGraphItem::loadFromJSON(objCopy);
}

void ACEILeverGraphItem::saveToJSON(QJsonObject &obj) const
{
    AbstractNodeGraphItem::saveToJSON(obj);

    // Replace fake node type with ours
    obj["type"] = CustomNodeType;

    obj["lever"] = mLever ? mLever->name() : QString();
    obj["lever_type"] = mLever ? mLever->getType() : QString();

    obj["light_left"] = mLeftLight ? mLeftLight->name() : QString();
    obj["light_right"] = mRightLight ? mRightLight->name() : QString();
}

void ACEILeverGraphItem::onLeverDestroyed()
{
    mLever = nullptr;
    mLeverIface = nullptr;
    updateLeverTooltip();
    emit leverChanged(mLever);
}

void ACEILeverGraphItem::onLightDestroyed()
{
    if(sender() == mLeftLight)
        setLeftLight(nullptr);
    else if(sender() == mRightLight)
        setRightLight(nullptr);
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
