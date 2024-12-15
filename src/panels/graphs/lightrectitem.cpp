/**
 * src/panels/graphs/lightrectitem.cpp
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

#include "lightrectitem.h"

#include "../../objects/simple_activable/lightbulbobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../panelscene.h"
#include "../../views/modemanager.h"

#include <QPainter>

#include <QJsonObject>

LightRectItem::LightRectItem()
    : AbstractPanelItem()
{

}

LightRectItem::~LightRectItem()
{
    setLightObject(nullptr);
}

QString LightRectItem::itemType() const
{
    return ItemType;
}

bool LightRectItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!AbstractPanelItem::loadFromJSON(obj, mgr))
        return false;

    // Light
    auto model = mgr->modelForType(LightBulbObject::Type);
    if(model)
    {
        const QString objName = obj.value("light").toString();
        AbstractSimulationObject *activationObj = model->getObjectByName(objName);
        setLightObject(static_cast<LightBulbObject *>(activationObj));
    }
    else
        setLightObject(nullptr);

    // Rect
    QRectF r;
    r.setWidth(obj.value("width").toDouble(20));
    r.setHeight(obj.value("height").toDouble(20));
    setRect(r);

    // Rotation
    const double rot = obj.value("rotation").toDouble(0);
    setRotation(qBound(-180.0, rot, +180.0));

    // Color
    QColor c = QColor::fromString(obj.value("color").toString());
    setColor(c.isValid() ? c : Qt::magenta);

    return true;
}

void LightRectItem::saveToJSON(QJsonObject &obj) const
{
    AbstractPanelItem::saveToJSON(obj);

    // Light
    obj["light"] = mLightObject ? mLightObject->name() : QString();

    // Rect
    obj["width"] = mRect.width();
    obj["height"] = mRect.height();

    // Rotation
    obj["rotation"] = rotation();

    // Color
    obj["color"] = mColor.name(QColor::HexRgb);
}

QRectF LightRectItem::rect() const
{
    return mRect;
}

void LightRectItem::setRect(const QRectF &newRect)
{
    if (mRect == newRect)
        return;
    prepareGeometryChange();
    mRect = newRect;
    update();

    emit rectChanged();
    if(panelScene())
        panelScene()->modeMgr()->setFileEdited();
}

QRectF LightRectItem::boundingRect() const
{
    return mRect;
}

void LightRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(mActive)
    {
        painter->fillRect(mRect, mColor);
    }
    else if(panelScene()->modeMgr()->mode() == FileMode::Editing)
    {
        QPen pen;
        pen.setWidth(4);
        pen.setColor(isSelected() ? Qt::magenta : Qt::blue);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        const double halfW = pen.widthF() / 2.0;

        QRectF r = mRect;
        r.adjust(halfW, halfW, -halfW, -halfW);
        painter->drawRect(r);
    }
}

QPainterPath LightRectItem::opaqueArea() const
{
    if(mActive)
        return shape();
    return QGraphicsObject::opaqueArea();
}

bool LightRectItem::active() const
{
    return mActive;
}

void LightRectItem::setActive(bool newActive)
{
    if(mActive == newActive)
        return;

    mActive = newActive;
    update();
}

QColor LightRectItem::color() const
{
    return mColor;
}

void LightRectItem::setColor(const QColor &newColor)
{
    if(mColor == newColor)
        return;

    mColor = newColor;
    update();

    emit colorChanged();
    if(panelScene())
        panelScene()->modeMgr()->setFileEdited();
}

LightBulbObject *LightRectItem::lightObject() const
{
    return mLightObject;
}

void LightRectItem::setLightObject(LightBulbObject *newLightObject)
{
    if(mLightObject == newLightObject)
        return;

    if(mLightObject)
    {
        disconnect(mLightObject, &LightBulbObject::stateChanged,
                   this, &LightRectItem::onLightStateChanged);
    }

    mLightObject = newLightObject;

    if(mLightObject)
    {
        connect(mLightObject, &LightBulbObject::stateChanged,
                this, &LightRectItem::onLightStateChanged);
    }

    emit lightChanged();
    if(panelScene())
        panelScene()->modeMgr()->setFileEdited();
    onLightStateChanged();
}

void LightRectItem::onLightStateChanged()
{
    setActive(mLightObject &&
              mLightObject->state() == LightBulbObject::State::On);
}
