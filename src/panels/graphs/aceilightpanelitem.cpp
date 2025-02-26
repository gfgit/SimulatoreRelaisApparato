/**
 * src/panels/graphs/aceilightpanelitem.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#include "aceilightpanelitem.h"
#include "../panelscene.h"

#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../views/modemanager.h"

#include <QPainter>
#include <QPen>

#include <QJsonObject>

ACEILightPanelItem::ACEILightPanelItem()
    : SnappablePanelItem()
{

}

ACEILightPanelItem::~ACEILightPanelItem()
{

}

QString ACEILightPanelItem::itemType() const
{
    return ItemType;
}

QRectF ACEILightPanelItem::boundingRect() const
{
    return QRectF(0, 0, ItemWidth, ItemHeight);
}

void ACEILightPanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QRectF br = boundingRect();

    // Background
    painter->fillRect(br, isSelected() ? SelectedBackground : qRgb(0x7F, 0x7F, 0x7F));

    const QPointF center = br.center();

    constexpr QRgb BorderColor = qRgb(97, 97, 97);

    // Draw dark gray border around
    QPen borderPen;
    borderPen.setWidth(2);
    borderPen.setColor(BorderColor);
    borderPen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));

    // Draw base circle below light
    QRectF circle;
    circle.setSize(QSizeF(baseCircleRadius * 2,
                          baseCircleRadius * 2));
    circle.moveCenter(center);
    painter->setBrush(Qt::lightGray);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle);

    // Draw Light
    circle.setSize(QSizeF(lightCircleRadius * 2,
                          lightCircleRadius * 2));

    borderPen.setWidth(3);
    painter->setPen(borderPen);

    if(mLight && mLight->state() == LightBulbObject::State::On)
        painter->setBrush(mLightColor);
    else
        painter->setBrush(Qt::NoBrush);

    circle.moveCenter(center);
    painter->drawEllipse(circle);

    // Draw Light name
    const QString lightName = mLight ? mLight->name() : tr("NULL");

    QRectF textRect;
    textRect.setLeft(10);
    textRect.setRight(br.width() - 10.0);
    textRect.setTop(center.y() + baseCircleRadius);
    textRect.setBottom(br.bottom() - 4.0);

    Qt::Alignment textAlign = Qt::AlignLeft;

    QFont f;
    f.setPointSizeF(18.0);
    f.setBold(true);

    QFontMetrics metrics(f, painter->device());
    double width = metrics.horizontalAdvance(lightName, QTextOption(textAlign));
    if(width > textRect.width())
    {
        f.setPointSizeF(f.pointSizeF() * textRect.width() / width * 0.9);
    }

    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);

    painter->setFont(f);
    painter->drawText(textRect, textAlign, lightName);
}

bool ACEILightPanelItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!SnappablePanelItem::loadFromJSON(obj, mgr))
        return false;

    // Light
    auto lightModel = mgr->modelForType(LightBulbObject::Type);

    const QString lightObjName = obj.value("light_central").toString();
    LightBulbObject *light = nullptr;
    if(lightModel)
        light = static_cast<LightBulbObject *>(lightModel->getObjectByName(lightObjName));

    setLight(light);

    QColor c = QColor::fromString(obj.value("light_central_color").toString());
    if(!c.isValid())
        c = Qt::yellow;

    setLightColor(c);

    return true;
}

void ACEILightPanelItem::saveToJSON(QJsonObject &obj) const
{
    SnappablePanelItem::saveToJSON(obj);

    // Light
    obj["light_central"] = mLight ? mLight->name() : QString();
    obj["light_central_color"] = mLightColor.name(QColor::HexRgb);
}

void ACEILightPanelItem::setLight(LightBulbObject *newLight)
{
    if(mLight == newLight)
        return;

    if(mLight)
    {
        disconnect(mLight, &LightBulbObject::stateChanged,
                   this, &ACEILightPanelItem::triggerUpdate);
        disconnect(mLight, &LightBulbObject::destroyed,
                   this, &ACEILightPanelItem::onLightDestroyed);
    }

    mLight = newLight;

    if(mLight)
    {
        connect(mLight, &LightBulbObject::stateChanged,
                this, &ACEILightPanelItem::triggerUpdate);
        connect(mLight, &LightBulbObject::destroyed,
                this, &ACEILightPanelItem::onLightDestroyed);
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

void ACEILightPanelItem::setLightColor(const QColor &newLightColor)
{
    if(mLightColor == newLightColor)
        return;

    mLightColor = newLightColor;

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

void ACEILightPanelItem::onLightDestroyed()
{
    if(sender() == mLight)
        setLight(nullptr);
}
