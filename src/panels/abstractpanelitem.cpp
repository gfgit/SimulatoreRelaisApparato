/**
 * src/panels/abstractpanelitem.cpp
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

#include "abstractpanelitem.h"

#include "panelscene.h"

#include "../views/modemanager.h"

#include <QPainter>
#include <QFont>

#include <QGraphicsSceneMouseEvent>

#include <QJsonObject>

AbstractPanelItem::AbstractPanelItem()
    : QGraphicsObject()
{
    setFlag(ItemSendsGeometryChanges, true);
}

QRectF AbstractPanelItem::boundingRect() const
{
    return QRectF(0, 0, TileLocation::Size, TileLocation::Size);
}

void AbstractPanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
    {
        // We are in item selection mode
        if(isSelected())
            painter->fillRect(boundingRect(), qRgb(180, 255, 255));
    }
}

QPainterPath AbstractPanelItem::opaqueArea() const
{
    return shape();
}

void AbstractPanelItem::triggerUpdate()
{
    update();
}

void AbstractPanelItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->mode() == FileMode::Editing)
    {
        if(ev->button() == Qt::LeftButton)
        {
            s->requestEditNode(this);
            return;
        }
    }

    QGraphicsObject::mouseDoubleClickEvent(ev);
}

QVariant AbstractPanelItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    PanelScene *s = panelScene();
    switch (change)
    {
    case GraphicsItemChange::ItemPositionChange:
    {
        QPointF newPos = value.toPointF();

        if(s && newPos != pos())
        {
            s->updateItemLocation(this);
        }

        return newPos;
    }
    case GraphicsItemChange::ItemSelectedHasChanged:
    {
        s->onItemSelected(this, isSelected());
        break;
    }
    default:
        break;
    }

    return QGraphicsObject::itemChange(change, value);
}

PanelScene *AbstractPanelItem::panelScene() const
{
    return qobject_cast<PanelScene *>(scene());
}

bool AbstractPanelItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(obj.value("type") != itemType())
        return false;

    setObjectName(obj.value("name").toString());

    QPointF pt(obj.value("x").toDouble(), obj.value("y").toDouble());
    setPos(pt);

    return true;
}

void AbstractPanelItem::saveToJSON(QJsonObject &obj) const
{
    obj["type"] = itemType();
    obj["name"] = objectName();

    obj["x"] = x();
    obj["y"] = y();
}
