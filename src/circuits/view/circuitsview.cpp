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

#include "../edit/nodeeditfactory.h"
#include "../graphs/abstractnodegraphitem.h"
#include "../nodes/abstractcircuitnode.h"

#include "circuitnodeobjectreplacedlg.h"
#include "../../utils/itemobjectreplacedlg_impl.hpp"

#include "../../views/modemanager.h"

#include <QKeyEvent>

#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

#include <QPainter>
#include <QSvgGenerator>

#include <QPointer>

CircuitsView::CircuitsView(ViewManager *viewMgr, QWidget *parent)
    : ZoomGraphView{parent}
    , mViewMgr(viewMgr)
{
    setDragMode(RubberBandDrag);
    setRubberBandSelectionMode(Qt::IntersectsItemShape);
}

CircuitScene *CircuitsView::circuitScene() const
{
    if(!scene())
        return nullptr;
    return static_cast<CircuitScene *>(scene());
}

void CircuitsView::addNodeAtLocation(NodeEditFactory *editFactory, const QString &nodeType, const TileLocation &tileHint)
{
    if(!circuitScene())
        return;

    const auto needsName = editFactory->needsName(nodeType);
    QString name;
    if(needsName == NodeEditFactory::NeedsName::Always)
    {
        name = QInputDialog::getText(this,
                                     tr("Choose Item Name"),
                                     tr("Name:"));
        if(name.isEmpty())
            return;
    }

    auto item = editFactory->createItem(nodeType, circuitScene());

    if(needsName == NodeEditFactory::NeedsName::Always)
        item->getAbstractNode()->setObjectName(name);

    // Set location hint, then scene might change it if not free
    item->setLocation(tileHint);

    // Add node to scene
    circuitScene()->addNode(item);

    ensureVisible(item);
}

void CircuitsView::setMode(FileMode newMode, FileMode oldMode)
{
    if(!circuitScene())
        return;

    // Changing mode can change scene rect. Try to keep current viewport position
    const QPoint vpCenter(viewport()->width() / 2, viewport()->height() / 2);
    const QPointF sceneVpCenter = mapToScene(vpCenter);

    circuitScene()->setMode(newMode, oldMode);

    centerOn(sceneVpCenter);
}

void CircuitsView::batchNodeEdit()
{
    if(!circuitScene() || !circuitScene()->hasMultipleNodesSelected() || !circuitScene()->areSelectedNodesSameType())
        return;

    if(circuitScene()->modeMgr()->mode() != FileMode::Editing)
        return;

    QVector<AbstractNodeGraphItem *> items = circuitScene()->getSelectedNodes();

    CircuitNodeObjectReplaceDlg::batchNodeEdit(items, mViewMgr, this);

    QMetaObject::invokeMethod(this, [this, items]()
    {
        ensureItemsSelected(items);
    }, Qt::QueuedConnection);
}

void CircuitsView::batchObjectReplace()
{
    if(!circuitScene())
        return;

    if(circuitScene()->modeMgr()->mode() != FileMode::Editing)
        return;

    QVector<AbstractNodeGraphItem *> items = circuitScene()->getSelectedNodes();

    QPointer<CircuitNodeObjectReplaceDlg> dlg = new CircuitNodeObjectReplaceDlg(mViewMgr,
                                                                                items,
                                                                                this);
    dlg->exec();
    if(dlg)
        delete dlg;

    QMetaObject::invokeMethod(this, [this, items]()
    {
        ensureItemsSelected(items);
    }, Qt::QueuedConnection);
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
            else if(ev->key() == Qt::Key_D && ev->modifiers() & Qt::ControlModifier)
            {
                // Batch Edit (Ctrl + D)
                if(ev->modifiers() & Qt::ShiftModifier)
                    batchNodeEdit();
                else
                    batchObjectReplace();
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
                return;
            }
        }

        if(ev->modifiers() == Qt::NoModifier &&
                circuitScene()->modeMgr()->editingSubMode() == EditingSubMode::Default)
        {
            // Try with letter shortcut

            if(ev->key() >= Qt::Key_A && ev->key() <= Qt::Key_Z && !ev->text().isEmpty())
            {
                // It's a letter
                const QChar letter = ev->text().at(0);
                const QString nodeType = circuitScene()->modeMgr()->circuitFactory()->typeForShortcutLetter(letter);

                if(!nodeType.isEmpty())
                {
                    // Add node to cursor pos
                    const TileLocation tileHint = TileLocation::fromPointFloor(getTargetScenePos());
                    addNodeAtLocation(circuitScene()->modeMgr()->circuitFactory(),
                                      nodeType,
                                      tileHint);
                    return;
                }
                else if(letter.toUpper() == 'C')
                {
                    circuitScene()->startEditNewCable();
                    return;
                }
            }
        }
    }

    if(ev->key() == Qt::Key_R && ev->modifiers() == Qt::ControlModifier)
    {
        // Render scene to SVG (Ctrl + R)
        QString fileName = QFileDialog::getSaveFileName(this, tr("Render To SVG"),
                                                        QLatin1String("%1.svg").arg(circuitScene()->circuitSheetName()),
                                                        tr("Scalable Vector Graphics (*.svg)"));
        if(!fileName.isEmpty())
        {
            renderToSVG(fileName);
        }
        return;
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

void CircuitsView::renderToSVG(const QString &fileName)
{
    QSvgGenerator svg(QSvgGenerator::SvgVersion::SvgTiny12);
    svg.setTitle(circuitScene()->circuitSheetName());
    svg.setDescription(tr("Simulatore Relais Apparato"));
    svg.setFileName(fileName);

    QRectF bounds = circuitScene()->itemsBoundingRect();
    bounds.adjust(-TileLocation::Size, -TileLocation::Size,
                  TileLocation::Size, TileLocation::Size);
    svg.setSize(bounds.size().toSize());

    QPainter p(&svg);

    p.translate(-bounds.topLeft());
    circuitScene()->render(&p);
}

void CircuitsView::ensureItemsSelected(const QVector<AbstractNodeGraphItem *> &items)
{
    // NOTE: sometime when closing dialog with Esc key, the event is handled
    // by scene too which disables selection. In that case restore it manually.

    if(!circuitScene() || circuitScene()->modeMgr()->mode() != FileMode::Editing)
        return;

    if(circuitScene()->modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
        return; // Everything ok

    circuitScene()->modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);

    for(AbstractNodeGraphItem *item : items)
        item->setSelected(true);
}
