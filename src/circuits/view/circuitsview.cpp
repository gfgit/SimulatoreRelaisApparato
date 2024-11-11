/**
 * src/circuits/view/circuitsview.cpp
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

#include "circuitsview.h"

#include "../circuitscene.h"

#include "../../views/modemanager.h"

#include <QKeyEvent>

#include <QMessageBox>

CircuitsView::CircuitsView(QWidget *parent)
    : ZoomGraphView{parent}
{
    setDragMode(RubberBandDrag);
}

CircuitScene *CircuitsView::circuitScene() const
{
    if(!scene())
        return nullptr;
    return static_cast<CircuitScene *>(scene());
}

void CircuitsView::keyPressEvent(QKeyEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->modeMgr()->mode() == FileMode::Editing)
    {
        if(s->modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
        {
            if(ev->matches(QKeySequence::Delete))
            {
                deleteSelectedItems();
                return;
            }
        }

        if(ev->matches(QKeySequence::Paste))
        {
            // Paste on mouse location
            const TileLocation tileHint = TileLocation::fromPointFloor(getTargetScenePos());

            TileLocation topLeft, bottomRight;
            if(s->tryPasteItems(tileHint, topLeft, bottomRight))
            {
                QRectF pastedArea;
                pastedArea.setTopLeft(topLeft.toPoint());

                // Consider bottom right corner of bottom right tile
                // So +1 dx and +1 dy
                pastedArea.setBottomRight(bottomRight.adjusted(1, 1).toPoint());
                ensureVisible(pastedArea);
            }
        }
    }

    ZoomGraphView::keyPressEvent(ev);
}

void CircuitsView::deleteSelectedItems()
{
    CircuitScene *s = circuitScene();
    if(!s)
        return;

    if(s->selectedItems().isEmpty())
        return;

    int ret = QMessageBox::question(this,
                                    tr("Delete Items"),
                                    tr("Delete selected items?"
                                       "Are you sure?"));
    if(ret == QMessageBox::Yes)
    {
        s->removeSelectedItems();
    }
}
