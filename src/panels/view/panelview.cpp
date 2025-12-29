/**
 * src/panels/view/panelview.cpp
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

#include "panelview.h"

#include "../panelscene.h"

#include "panelitemobjectreplacedlg.h"
#include "../../utils/itemobjectreplacedlg_impl.hpp"

#include "../../views/modemanager.h"

#include <QKeyEvent>

#include <QMessageBox>

PanelView::PanelView(ViewManager *viewMgr, QWidget *parent)
    : ZoomGraphView{parent}
    , mViewMgr(viewMgr)
{
    setDragMode(RubberBandDrag);
}

PanelScene *PanelView::panelScene() const
{
    if(!scene())
        return nullptr;
    return static_cast<PanelScene *>(scene());
}

void PanelView::batchNodeEdit()
{
    if(!panelScene() || !panelScene()->areSelectedNodesSameType())
        return;

    if(panelScene()->modeMgr()->mode() != FileMode::Editing)
        return;

    QVector<AbstractPanelItem *> items = panelScene()->getSelectedItems();

    PanelItemObjectReplaceDlg::batchNodeEdit(items, mViewMgr, this);
}

void PanelView::batchObjectReplace()
{
    if(!panelScene())
        return;

    if(panelScene()->modeMgr()->mode() != FileMode::Editing)
        return;

    QVector<AbstractPanelItem *> items = panelScene()->getSelectedItems();

    QPointer<PanelItemObjectReplaceDlg> dlg = new PanelItemObjectReplaceDlg(mViewMgr,
                                                                            items,
                                                                            this);
    dlg->exec();
    if(dlg)
        delete dlg;
}

void PanelView::keyPressEvent(QKeyEvent *ev)
{
    PanelScene *s = panelScene();
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
        else if(ev->key() == Qt::Key_E && ev->modifiers() & Qt::ControlModifier)
        {
            // Batch Edit (Ctrl + E)
            if(ev->modifiers() & Qt::ShiftModifier)
                batchNodeEdit();
            else
                batchObjectReplace();
            return;
        }

        if(ev->matches(QKeySequence::Paste))
        {
            // Paste on mouse location
            const QPointF tileHint = getTargetScenePos();

            QPointF topLeft, bottomRight;
            if(s->tryPasteItems(tileHint, topLeft, bottomRight))
            {
                QRectF pastedArea;
                pastedArea.setTopLeft(topLeft);

                // Consider bottom right corner of bottom right tile
                // So +1 dx and +1 dy
                // TODO
                pastedArea.setBottomRight(bottomRight);
                ensureVisible(pastedArea);
            }
        }
    }

    ZoomGraphView::keyPressEvent(ev);
}

void PanelView::deleteSelectedItems()
{
    PanelScene *s = panelScene();
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
