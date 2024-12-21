/**
 * src/panels/snappablepanelitem.cpp
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

#include "snappablepanelitem.h"

#include "panelscene.h"

#include <QGraphicsSceneMouseEvent>

SnappablePanelItem::SnappablePanelItem(QObject *parent)
    : AbstractPanelItem()
{

}

void SnappablePanelItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    // Disable snap with Shift key
    mSnapEnabled = !ev->modifiers().testFlag(Qt::ShiftModifier);

    AbstractPanelItem::mousePressEvent(ev);
}

void SnappablePanelItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    // Disable snap with Shift key
    mSnapEnabled = !ev->modifiers().testFlag(Qt::ShiftModifier);

    AbstractPanelItem::mouseMoveEvent(ev);
}

void SnappablePanelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    // Snap only during mouse drag
    mSnapEnabled = false;

    AbstractPanelItem::mouseReleaseEvent(ev);
}

QVariant SnappablePanelItem::itemChange(GraphicsItemChange change, const QVariant &value)
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

            const QRectF oldRect = boundingRect().translated(pos());
            QRectF newRect = boundingRect().translated(newPos);

            // Do not snap on ourselves
            s->unregisterSnap(oldRect);

            // Snap only if not other item is currently selected
            const auto selection = s->selectedItems();
            if(selection.size() > 1 || (selection.size() == 1 && selection.at(0) != this))
                mSnapEnabled = false;

            // Snap only if Shift is not pressed
            if(mSnapEnabled)
            {
                // Calculate best snap
                const PanelScene::SnapResult snapPos = s->getSnapFor(newPos);
                const PanelScene::SnapResult snapCorner = s->getSnapFor(newRect.bottomRight());

                const QPointF deltaPos = snapPos.pt - newPos;
                const QPointF deltaCorner = snapCorner.pt - newRect.bottomRight();

                if(deltaPos.x() >= 10 || deltaCorner.x() >= 10)
                    qt_noop();

                if(snapCorner.foundX && (!snapPos.foundX || qAbs(deltaCorner.x()) <= qAbs(deltaPos.x())))
                {
                    newPos.rx() += deltaCorner.x();
                }
                else if(snapPos.foundX && (!snapCorner.foundX || qAbs(deltaPos.x()) < qAbs(deltaCorner.x())))
                {
                    newPos.rx() += deltaPos.x();
                }

                if(snapCorner.foundY && (!snapPos.foundY || qAbs(deltaCorner.y()) <= qAbs(deltaPos.y())))
                {
                    newPos.ry() += deltaCorner.y();
                }
                else if(snapPos.foundY && (!snapCorner.foundY || qAbs(deltaPos.y()) < qAbs(deltaCorner.y())))
                {
                    newPos.ry() += deltaPos.y();
                }

                newRect = boundingRect().translated(newPos);
            }

            // Update snap register
            s->registerSnap(newRect);
        }

        return newPos;
    }
    default:
        break;
    }

    return AbstractPanelItem::itemChange(change, value);
}
