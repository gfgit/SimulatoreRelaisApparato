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

void AbstractPanelItem::triggerUpdate()
{
    update();
}

void AbstractPanelItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    // Sometimes we receive clicks even if out of node tile
    // In those cases do not start moving item or rotate it!
    PanelScene *s = panelScene();
    if(s && s->mode() == FileMode::Editing && boundingRect().contains(ev->pos()))
    {
        const EditingSubMode subMode = s->modeMgr()->editingSubMode();

        if(subMode == EditingSubMode::Default)
        {
            if(ev->button() == Qt::LeftButton)
            {
                s->startMovingItem(this);
            }
        }
    }

    QGraphicsObject::mousePressEvent(ev);
}

void AbstractPanelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->mode() == FileMode::Editing)
    {
        const EditingSubMode subMode = s->modeMgr()->editingSubMode();

        if(subMode == EditingSubMode::SingleItemMove)
        {
            // After move has ended we go back to last valid location
            s->endMovingItem();
        }
        else if(subMode == EditingSubMode::ItemSelection)
        {
            s->endSelectionMove();
        }
    }

    QGraphicsObject::mouseReleaseEvent(ev);
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
        // TODO: custom snap to align to other items
        QPointF newPos = value.toPointF();

        if(newPos != pos() && s && s->modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        {
            // For item selection mode we bypass normal logic
            if(!s->updateItemLocation(this))
            {
                // New position was not free
                // Reset to old position
                return pos();
            }
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

bool AbstractPanelItem::loadFromJSON(const QJsonObject &obj)
{
    if(obj.value("type") != itemType())
        return false;

    setObjectName(obj.value("name").toString());

    QPointF pt(obj.value("x").toInt(), obj.value("y").toInt());
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
