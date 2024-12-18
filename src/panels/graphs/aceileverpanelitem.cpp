/**
 * src/panels/graphs/aceileverpanelitem.cpp
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

#include "aceileverpanelitem.h"
#include "../panelscene.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../objects/interfaces/leverinterface.h"

#include "../../objects/lever/acei/aceileverobject.h"

//TODO: remove BEM
#include "../../objects/lever/bem/bemleverobject.h"

#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../views/modemanager.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

#include <QtMath>

#include <QJsonObject>

ACEILeverPanelItem::ACEILeverPanelItem()
    : AbstractPanelItem()
{
    updateLeverTooltip();
}

ACEILeverPanelItem::~ACEILeverPanelItem()
{
    setLeftLight(nullptr);
    setRightLight(nullptr);
    setLever(nullptr);
}

QString ACEILeverPanelItem::itemType() const
{
    return ItemType;
}

QRectF ACEILeverPanelItem::boundingRect() const
{
    return QRectF(0, 0, ItemWidth, ItemHeight);
}

void ACEILeverPanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QRectF br = boundingRect();

    if(isSelected())
        painter->fillRect(br, SelectedBackground);

    // Zero is vertical up, so cos/sin are swapped
    // Also returned angle must be inverted to be clockwise
    const double angleRadiants = -qDegreesToRadians(mLeverIface ? mLeverIface->angle() : 0);
    const QPointF delta(qSin(angleRadiants),
                        qCos(angleRadiants));

    const QPointF center = br.center();

    const QPointF leverCenter(center.x(), center.y());

    constexpr QRgb BorderColor = qRgb(97, 97, 97);

    // Draw dark gray border around
    QPen borderPen;
    borderPen.setWidth(3);
    borderPen.setColor(BorderColor);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());

    // Draw lights
    QRectF circle;
    circle.setSize(QSizeF(lightCircleRadius * 2,
                          lightCircleRadius * 2));

    if(mLeftLight)
    {
        if(mLeftLight->state() == LightBulbObject::State::On)
            painter->setBrush(mLeftLightColor);
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(lightOffset, lightOffset));
        painter->drawEllipse(circle);
    }

    if(mRightLight)
    {
        if(mRightLight->state() == LightBulbObject::State::On)
            painter->setBrush(mRightLightColor);
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(br.width() - lightOffset, lightOffset));
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
    QColor color = qRgb(77, 77, 77); // Medium Dark gray
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
    textRect.setRight(br.width() - 10.0);
    textRect.setTop(leverCenter.y() + baseCircleRadius);
    textRect.setBottom(br.bottom() - 4.0);

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

void ACEILeverPanelItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing
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

    AbstractPanelItem::mousePressEvent(ev);
}

void ACEILeverPanelItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    const QPointF center = boundingRect().center();

    const QPointF leverCenter(center.x(), center.y());

    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing
            && mLeverIface)
    {
        if(ev->buttons() & Qt::LeftButton)
        {
            QPointF delta = ev->pos() - mLastMousePos;
            if(delta.manhattanLength() > 2)
            {
                // Calculate angle
                delta = leverCenter - ev->pos();

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

    AbstractPanelItem::mouseMoveEvent(ev);
}

void ACEILeverPanelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing)
    {
        // We don't care about button
        // Also sometimes there are already no buttons
        if(mLeverIface)
            mLeverIface->setPressed(false);
    }

    AbstractPanelItem::mouseReleaseEvent(ev);
}

void ACEILeverPanelItem::updateLeverTooltip()
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

QColor ACEILeverPanelItem::rightLightColor() const
{
    return mRightLightColor;
}

void ACEILeverPanelItem::setRightLightColor(const QColor &newRightLightColor)
{
    if(mRightLightColor == newRightLightColor)
        return;

    mRightLightColor = newRightLightColor;

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

QColor ACEILeverPanelItem::leftLightColor() const
{
    return mLeftLightColor;
}

void ACEILeverPanelItem::setLeftLightColor(const QColor &newLeftLightColor)
{
    if(mLeftLightColor == newLeftLightColor)
        return;

    mLeftLightColor = newLeftLightColor;

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

LightBulbObject *ACEILeverPanelItem::leftLight() const
{
    return mLeftLight;
}

void ACEILeverPanelItem::setLeftLight(LightBulbObject *newLeftLight)
{
    if(mLeftLight == newLeftLight)
        return;

    if(mLeftLight)
    {
        disconnect(mLeftLight, &LightBulbObject::stateChanged,
                   this, &ACEILeverPanelItem::triggerUpdate);
        disconnect(mLeftLight, &LightBulbObject::destroyed,
                   this, &ACEILeverPanelItem::onLightDestroyed);
    }

    mLeftLight = newLeftLight;

    if(mLeftLight)
    {
        connect(mLeftLight, &LightBulbObject::stateChanged,
                this, &ACEILeverPanelItem::triggerUpdate);
        connect(mLeftLight, &LightBulbObject::destroyed,
                this, &ACEILeverPanelItem::onLightDestroyed);
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

LightBulbObject *ACEILeverPanelItem::rightLight() const
{
    return mRightLight;
}

void ACEILeverPanelItem::setRightLight(LightBulbObject *newRightLight)
{
    if(mRightLight == newRightLight)
        return;

    if(mRightLight)
    {
        disconnect(mRightLight, &LightBulbObject::stateChanged,
                   this, &ACEILeverPanelItem::triggerUpdate);
        disconnect(mRightLight, &LightBulbObject::destroyed,
                   this, &ACEILeverPanelItem::onLightDestroyed);
    }

    mRightLight = newRightLight;

    if(mRightLight)
    {
        connect(mRightLight, &LightBulbObject::stateChanged,
                this, &ACEILeverPanelItem::triggerUpdate);
        connect(mRightLight, &LightBulbObject::destroyed,
                this, &ACEILeverPanelItem::onLightDestroyed);
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

AbstractSimulationObject *ACEILeverPanelItem::lever() const
{
    return mLever;
}

void ACEILeverPanelItem::setLever(AbstractSimulationObject *newLever)
{
    // TODO: remove BEM
    if(newLever && newLever->getType() != ACEILeverObject::Type && newLever->getType() != BEMLeverObject::Type)
        return;

    if(mLever)
    {
        disconnect(mLever, &AbstractSimulationObject::destroyed,
                   this, &ACEILeverPanelItem::onLeverDestroyed);
        disconnect(mLever, &AbstractSimulationObject::stateChanged,
                   this, &ACEILeverPanelItem::triggerUpdate);
        disconnect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ACEILeverPanelItem::onInterfacePropertyChanged);
        disconnect(mLever, &AbstractSimulationObject::settingsChanged,
                   this, &ACEILeverPanelItem::triggerUpdate);
        mLeverIface = nullptr;
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &AbstractSimulationObject::destroyed,
                this, &ACEILeverPanelItem::onLeverDestroyed);
        connect(mLever, &AbstractSimulationObject::stateChanged,
                this, &ACEILeverPanelItem::triggerUpdate);
        connect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ACEILeverPanelItem::onInterfacePropertyChanged);
        connect(mLever, &AbstractSimulationObject::settingsChanged,
                this, &ACEILeverPanelItem::triggerUpdate);

        mLeverIface = mLever->getInterface<LeverInterface>();
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    updateLeverTooltip();

    emit leverChanged(mLever);
}

bool ACEILeverPanelItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!AbstractPanelItem::loadFromJSON(obj, mgr))
        return false;

    const QString leverName = obj.value("lever").toString();
    const QString leverType = obj.value("lever_type").toString();
    auto model = mgr->modelForType(leverType);

    if(model)
        setLever(model->getObjectByName(leverName));
    else
        setLever(nullptr);

    auto lightModel = mgr->modelForType(LightBulbObject::Type);
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

    // Color
    QColor c = QColor::fromString(obj.value("light_left_color").toString());
    setLeftLightColor(c.isValid() ? c : Qt::yellow);

    c = QColor::fromString(obj.value("light_right_color").toString());
    setRightLightColor(c.isValid() ? c : Qt::blue);

    return true;
}

void ACEILeverPanelItem::saveToJSON(QJsonObject &obj) const
{
    AbstractPanelItem::saveToJSON(obj);

    obj["lever"] = mLever ? mLever->name() : QString();
    obj["lever_type"] = mLever ? mLever->getType() : QString();

    obj["light_left"] = mLeftLight ? mLeftLight->name() : QString();
    obj["light_right"] = mRightLight ? mRightLight->name() : QString();

    // Color
    obj["light_left_color"] = mLeftLightColor.name(QColor::HexRgb);
    obj["light_right_color"] = mRightLightColor.name(QColor::HexRgb);
}

void ACEILeverPanelItem::onLeverDestroyed()
{
    mLever = nullptr;
    mLeverIface = nullptr;
    updateLeverTooltip();
    emit leverChanged(mLever);
}

void ACEILeverPanelItem::onLightDestroyed()
{
    if(sender() == mLeftLight)
        setLeftLight(nullptr);
    else if(sender() == mRightLight)
        setRightLight(nullptr);
}

void ACEILeverPanelItem::onInterfacePropertyChanged(const QString &ifaceName, const QString &propName, const QVariant &value)
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
