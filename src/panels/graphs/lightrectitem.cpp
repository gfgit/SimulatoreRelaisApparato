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
#include <QJsonArray>

LightRectItem::LightRectItem()
    : AbstractPanelItem()
{

}

LightRectItem::~LightRectItem()
{
    setLights({});
}

QString LightRectItem::itemType() const
{
    return ItemType;
}

bool LightRectItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!AbstractPanelItem::loadFromJSON(obj, mgr))
        return false;

    // Lights
    auto model = mgr->modelForType(LightBulbObject::Type);
    if(model)
    {
        if(obj.contains("light"))
        {
            // Compatibility: port from old files
            const QString objName = obj.value("light").toString();
            AbstractSimulationObject *activationObj = model->getObjectByName(objName);
            LightBulbObject *light = static_cast<LightBulbObject *>(activationObj);

            // Color
            QColor c = QColor::fromString(obj.value("color").toString());

            if(light)
            {
                LightEntry entry{light, c.isValid() ? c : Qt::magenta};
                setLights({entry});
            }
        }
        else
        {
            const QJsonArray lights = obj.value("lights").toArray();

            QVector<LightEntry> entries;
            entries.reserve(lights.size());
            for(const QJsonValue& v : lights)
            {
                const QJsonObject lightObj = v.toObject();

                const QString objName = lightObj.value("light").toString();
                AbstractSimulationObject *activationObj = model->getObjectByName(objName);
                LightBulbObject *light = static_cast<LightBulbObject *>(activationObj);

                // Color
                QColor c = QColor::fromString(lightObj.value("color").toString());

                if(light)
                {
                    LightEntry entry{light, c.isValid() ? c : Qt::magenta};
                    entries.append(entry);
                }
            }

            setLights(entries);
        }
    }
    else
        setLights({});

    // Rect
    QRectF r;
    r.setWidth(obj.value("width").toDouble(20));
    r.setHeight(obj.value("height").toDouble(20));
    setRect(r);

    // Rotation
    const double rot = obj.value("rotation").toDouble(0);
    setRotation(qBound(-180.0, rot, +180.0));

    return true;
}

void LightRectItem::saveToJSON(QJsonObject &obj) const
{
    AbstractPanelItem::saveToJSON(obj);

    // Lights
    QJsonArray lights;
    for(const LightEntry& entry : std::as_const(mLights))
    {
        QJsonObject lightObj;
        obj["light"] = entry.light->name();

        // Color
        obj["color"] = entry.color.name(QColor::HexRgb);

        lights.append(lightObj);
    }

    obj["lights"] = lights;

    // Rect
    obj["width"] = mRect.width();
    obj["height"] = mRect.height();

    // Rotation
    obj["rotation"] = rotation();
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
    if(panelScene()->modeMgr()->mode() == FileMode::Editing)
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
    else if(isActive())
    {
        // First active light wins
        for(const LightEntry& light : std::as_const(mLights))
        {
            if(light.light->state() == LightBulbObject::State::On)
            {
                painter->fillRect(mRect, light.color);
                break;
            }
        }
    }
}

QPainterPath LightRectItem::opaqueArea() const
{
    if(isActive())
        return shape();
    return QGraphicsObject::opaqueArea();
}

bool LightRectItem::active() const
{
    return mActive > 0;
}

void LightRectItem::onLightStateChanged(AbstractSimulationObject *obj)
{
    LightBulbObject *light = static_cast<LightBulbObject *>(obj);
    Q_ASSERT(std::find_if(mLights.cbegin(), mLights.cend(),
                          [light](const LightEntry& other) -> bool
    {
        return other.light == light;
    }) != mLights.cend());

    if(light->state() == LightBulbObject::State::On)
    {
        mActive++;
    }
    else
    {
        Q_ASSERT(mActive > 0);
        mActive--;
    }

    update();
}

void LightRectItem::onLightDestroyed(QObject *obj)
{
    LightBulbObject *light = static_cast<LightBulbObject *>(obj);
    auto it = std::find_if(mLights.begin(), mLights.end(),
                           [light](const LightEntry& other) -> bool
    {
        return other.light == light;
    });

    Q_ASSERT(it != mLights.end());

    disconnect(light, &LightBulbObject::stateChanged,
               this, &LightRectItem::onLightStateChanged);
    disconnect(light, &QObject::destroyed,
               this, &LightRectItem::onLightDestroyed);

    mLights.erase(it);

    emit lightsChanged();
    if(panelScene())
        panelScene()->modeMgr()->setFileEdited();
    update();
}

QVector<LightRectItem::LightEntry> LightRectItem::lights() const
{
    return mLights;
}

void LightRectItem::setLights(const QVector<LightEntry> &newLights)
{
    if(mLights == newLights)
        return;

    QSet<LightBulbObject *> uniqueLights;

    QVector<LightEntry> validatedLights;
    for(const LightEntry& entry : newLights)
    {
        if(!entry.light || uniqueLights.contains(entry.light))
            continue;

        validatedLights.append(entry);
        uniqueLights.insert(entry.light);

        auto it = std::find_if(mLights.begin(), mLights.end(),
                               [entry](const LightEntry& other) -> bool
        {
            return other.light == entry.light;
        });

        if(it != mLights.end())
        {
            mLights.erase(it);
            continue;
        }

        // Light is new, connect it
        if(entry.light->state() == LightBulbObject::State::On)
        {
            mActive++;
        }

        connect(entry.light, &LightBulbObject::stateChanged,
                this, &LightRectItem::onLightStateChanged);
        connect(entry.light, &QObject::destroyed,
                this, &LightRectItem::onLightDestroyed);
    }

    // Disconnect old lights
    if(!mLights.isEmpty())
    {
        for(const LightEntry& entry : std::as_const(mLights))
        {
            if(entry.light->state() == LightBulbObject::State::On)
            {
                Q_ASSERT(mActive > 0);
                mActive--;
            }

            disconnect(entry.light, &LightBulbObject::stateChanged,
                       this, &LightRectItem::onLightStateChanged);
            disconnect(entry.light, &QObject::destroyed,
                       this, &LightRectItem::onLightDestroyed);
        }
    }

    mLights = validatedLights;

    emit lightsChanged();
    if(panelScene())
        panelScene()->modeMgr()->setFileEdited();
    update();
}
