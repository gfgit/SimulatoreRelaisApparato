/**
 * src/panels/panelscene.cpp
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

#include "panelscene.h"

#include "view/panellistmodel.h"
#include "../views/modemanager.h"

#include "abstractpanelitem.h"

#include <QGraphicsPathItem>
#include <QPen>

#include <unordered_set>

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QPainter>

#include <QClipboard>
#include <QMimeData>

#include "edit/panelitemfactory.h"

PanelScene::PanelScene(PanelListModel *parent)
    : QGraphicsScene{parent}
{

}

PanelScene::~PanelScene()
{
    removeAllItems();
}

FileMode PanelScene::mode() const
{
    return panelsModel()->modeMgr()->mode();
}

ModeManager *PanelScene::modeMgr() const
{
    return panelsModel()->modeMgr();
}

void PanelScene::setMode(FileMode newMode, FileMode oldMode)
{
    // TODO: bring light rect items to front

    // Background changes between modes
    invalidate(QRectF(), BackgroundLayer);
}

void PanelScene::addNode(AbstractPanelItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    // Add item after having inserted it in the map
    addItem(item);

    setHasUnsavedChanges(true);
}

void PanelScene::removeNode(AbstractPanelItem *item)
{
    if(item == itemBeingMoved())
        endMovingItem();

    removeItem(item);

    setHasUnsavedChanges(true);
}

bool PanelScene::updateItemLocation(AbstractPanelItem *item)
{
    setHasUnsavedChanges(true);
    return true;
}

AbstractPanelItem *PanelScene::itemBeingMoved() const
{
    return mItemBeingMoved;
}

void PanelScene::startMovingItem(AbstractPanelItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::SingleItemMove);

    mItemBeingMoved = item;
    Q_ASSERT(mItemBeingMoved);

    if(panelsModel()->modeMgr()->mode() == FileMode::Editing)
        mItemBeingMoved->setFlag(QGraphicsItem::ItemIsMovable, true);
}

void PanelScene::endMovingItem()
{
    if(!mItemBeingMoved)
        return;

    mItemBeingMoved->setFlag(QGraphicsItem::ItemIsMovable, false);

    if(modeMgr()->editingSubMode() == EditingSubMode::SingleItemMove)
        modeMgr()->setEditingSubMode(EditingSubMode::Default);
}

void PanelScene::requestEditNode(AbstractPanelItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    if(mode() == FileMode::Editing)
        emit panelsModel()->itemEditRequested(item);
}

void PanelScene::onEditingSubModeChanged(EditingSubMode oldMode, EditingSubMode newMode)
{
    switch (oldMode)
    {
    case EditingSubMode::SingleItemMove:
        endMovingItem();
        break;
    case EditingSubMode::ItemSelection:
        endItemSelection();
        break;
    case EditingSubMode::Default:
    default:
        break;
    }

    switch (newMode)
    {
    case EditingSubMode::ItemSelection:
        startItemSelection();
        break;
    default:
        break;
    }
}

void PanelScene::startItemSelection()
{
    modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);

    allowItemSelection(true);
}

void PanelScene::endItemSelection()
{
    endSelectionMove();

    clearSelection();

    allowItemSelection(false);

    // Disable item selection if not already done
    if(modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
        modeMgr()->setEditingSubMode(EditingSubMode::Default);
}

void PanelScene::allowItemSelection(bool enabled)
{
    for(const auto& it : mItemMap)
    {
        AbstractPanelItem *item = it.second;
        item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    }
}

void PanelScene::onItemSelected(AbstractPanelItem *item, bool value)
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    if(value)
    {
        //TODO
    }
    else
    {

    }
}

void PanelScene::moveSelectionBy(int16_t dx, int16_t dy)
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    setHasUnsavedChanges(true);
}

void PanelScene::endSelectionMove()
{

}

void PanelScene::copySelectedItems()
{
    QJsonArray nodes;
    for(auto item : selectedItems())
    {
        QGraphicsObject *itemObj = item->toGraphicsObject();
        if(!itemObj)
            continue;

        AbstractPanelItem *panelItem = qobject_cast<AbstractPanelItem *>(itemObj);
        if(!panelItem)
            continue;

        QJsonObject nodeObj;
        panelItem->saveToJSON(nodeObj);
        nodes.append(nodeObj);
    }

    QJsonObject rootObj;
    rootObj["nodes"] = nodes;

    QByteArray value = QJsonDocument(rootObj).toJson(QJsonDocument::Compact);

    QMimeData *mime = new QMimeData;
    mime->setData(PanelMimeType, value);

    QGuiApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

bool PanelScene::tryPasteItems(const QPointF &tileHint,
                               QPointF &outTopLeft,
                               QPointF &outBottomRight)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if(!clipboard)
        return false;

    const QMimeData *mime = clipboard->mimeData(QClipboard::Clipboard);
    if(!mime)
        return false;

    QByteArray value = mime->data(PanelMimeType);
    if(value.isEmpty())
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(value);
    if(doc.isNull())
        return false;


    const QJsonObject rootObj = doc.object();

    return insertFragment(tileHint, rootObj,
                          modeMgr()->panelFactory(),
                          outTopLeft, outBottomRight);
}

void PanelScene::removeSelectedItems()
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    // Copy current selection
    const auto selectedItemsCopy = mSelectedItemPositions;

    // Clear selection
    clearSelection();
    mSelectedItemPositions.clear();

    // Remove previously selected items
    for(auto it : selectedItemsCopy)
    {
        AbstractPanelItem *item = it.first;
        removeNode(item);
    }
}

void PanelScene::selectAll()
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    for(const auto& it : mItemMap)
    {
        AbstractPanelItem *item = it.second;
        item->setSelected(true);
    }
}

void PanelScene::invertSelection()
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    const auto oldSelectionCopy = selectedItems();

    selectAll();

    // Now deselect previous items
    for(auto *item : oldSelectionCopy)
    {
        item->setSelected(false);
    }
}

bool PanelScene::insertFragment(const QPointF &tileHint,
                                const QJsonObject &fragmentRoot,
                                PanelItemFactory *factory,
                                QPointF &outTopLeft,
                                QPointF &outBottomRight)
{
    const QJsonArray nodes = fragmentRoot.value("nodes").toArray();
    const QJsonArray cables = fragmentRoot.value("cables").toArray();

    // Remove overlapping nodes/cables in fragment to be inserted
    FragmentData fragment;
    if(!checkFragment(nodes, cables, fragment))
        return false;

    // We have now skipped invalid items, check if anything is left
    if(fragment.validNodes.isEmpty())
        return false;

    // Find where to inster fragment, near provided tile hint
    QPointF fragmentOrigin = tileHint;

    // Fragment coordinates are still what use to be when copied
    // So they need to be adjusted
    // Calculate translation to apply to fragmen
    const QPointF delta = fragmentOrigin - fragment.topLeftLocation;

    // Ensure we are default mode (clears previous selection)
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    // Really paste items
    QVector<AbstractPanelItem *> pastedItems;
    pastedItems.reserve(fragment.validNodes.size());

    for(const QJsonObject& obj : fragment.validNodes)
    {
        const QString nodeType = obj.value("type").toString();
        if(nodeType.isEmpty())
            continue;

        // Create new node
        AbstractPanelItem *item = factory->createItem(nodeType, this);
        if(item)
        {
            if(!item->loadFromJSON(obj))
            {
                delete item;
                continue;
            }

            // Translate node
            QPointF pos = item->pos();
            pos += delta;
            item->setPos(pos);

            // Add node to scene
            addNode(item);

            pastedItems.append(item);
        }
    }

    // Now select all pasted items so user can move them
    modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);

    for(QGraphicsItem *item : std::as_const(pastedItems))
    {
        item->setSelected(true);
    }

    outTopLeft = fragment.topLeftLocation + delta;
    outBottomRight = fragment.bottomRightLocation + delta;

    return true;
}

bool PanelScene::checkFragment(const QJsonArray &nodes, const QJsonArray &cables, FragmentData &fragment)
{
    fragment.validNodes.reserve(nodes.size());

    const auto validTypes = modeMgr()->panelFactory()->getRegisteredTypes();

    // Check pasted nodes do not overlap each other
    for(const QJsonValue& v : nodes)
    {
        const QJsonObject obj = v.toObject();
        const QString nodeType = obj.value("type").toString();
        if(nodeType.isEmpty() || !validTypes.contains(nodeType))
            continue;

        QPointF pos(obj.value("x").toDouble(), obj.value("y").toDouble());

        fragment.validNodes.append(obj);

        fragment.trackFragmentBounds(pos);
    }

    return true;
}

QString PanelScene::panelLongName() const
{
    return mCircuitSheetLongName;
}

void PanelScene::setPanelLongName(const QString &newLongName)
{
    const QString trimmedName = newLongName.trimmed();

    if(mCircuitSheetLongName == trimmedName)
        return;

    mCircuitSheetLongName = trimmedName;
    emit longNameChanged(mCircuitSheetName, this);

    setHasUnsavedChanges(true);
}

QString PanelScene::panelName() const
{
    return mCircuitSheetName;
}

bool PanelScene::setPanelName(const QString &newName)
{
    const QString trimmedName = newName.trimmed();

    if(mCircuitSheetName == trimmedName)
        return true;

    bool isValid = panelsModel()->isNameAvailable(trimmedName);

    if(isValid)
    {
        // Do change name
        mCircuitSheetName = trimmedName;

        setHasUnsavedChanges(true);
    }

    emit nameChanged(mCircuitSheetName, this);

    return isValid;
}

bool PanelScene::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
}

void PanelScene::setHasUnsavedChanges(bool newHasUnsavedChanged)
{
    if(m_hasUnsavedChanges == newHasUnsavedChanged)
        return;

    m_hasUnsavedChanges = newHasUnsavedChanged;

    if(m_hasUnsavedChanges)
        panelsModel()->onSceneEdited();

    emit sceneEdited(m_hasUnsavedChanges);
}

void PanelScene::removeAllItems()
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    const auto itemsCopy = mItemMap;
    for(const auto& it : itemsCopy)
    {
        AbstractPanelItem *item = it.second;
        removeNode(item);
    }
    Q_ASSERT(mItemMap.size() == 0);
}

bool PanelScene::loadFromJSON(const QJsonObject &obj, PanelItemFactory *factory)
{
    removeAllItems();

    if(!obj.contains("cables") || !obj.contains("nodes"))
        return false;

    if(!setPanelName(obj.value("name").toString()))
        return false;

    setPanelLongName(obj.value("long_name").toString());

    const QJsonArray nodes = obj.value("nodes").toArray();

    for(const QJsonValue& v : nodes)
    {
        const QJsonObject nodeObj = v.toObject();
        const QString nodeType = nodeObj.value("type").toString();
        if(nodeType.isEmpty())
            continue;

        // Create new node
        AbstractPanelItem *item = factory->createItem(nodeType, this);
        if(item)
        {
            if(!item->loadFromJSON(nodeObj))
            {
                delete item;
                continue;
            }

            addNode(item);
        }
    }

    setHasUnsavedChanges(false);

    return true;
}

void PanelScene::saveToJSON(QJsonObject &obj) const
{
    obj["name"] = panelName();
    obj["long_name"] = panelLongName();

    QJsonArray nodes;
    for(const auto& it : mItemMap)
    {
        AbstractPanelItem *item = it.second;
        QJsonObject nodeObj;
        item->saveToJSON(nodeObj);
        nodes.append(nodeObj);
    }

    obj["nodes"] = nodes;
}

void PanelScene::keyReleaseEvent(QKeyEvent *e)
{
    bool consumed = true;

    switch (e->key())
    {
    case Qt::Key_Escape:
        modeMgr()->setEditingSubMode(EditingSubMode::Default);
        break;
    case Qt::Key_S:
        if(e->modifiers() == Qt::NoModifier && modeMgr()->mode() == FileMode::Editing)
            modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);
        break;
    default:
        consumed = false;
        break;
    }

    if(!consumed &&
            modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
    {
        consumed = true;

        if(e->matches(QKeySequence::Copy))
        {
            copySelectedItems();
        }
        else if(e->matches(QKeySequence::Cut))
        {
            copySelectedItems();
            removeSelectedItems();
        }
        else if(e->keyCombination() == QKeyCombination(Qt::ControlModifier, Qt::Key_I))
        {
            invertSelection();
        }
        else if(e->keyCombination() == QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_A))
        {
            clearSelection();
            mSelectedItemPositions.clear();
        }
        else
        {
            consumed = false;
        }

        // TODO: clear selection and invertselection
    }

    if(!consumed &&
            modeMgr()->mode() == FileMode::Editing)
    {
        consumed = true;

        if(e->matches(QKeySequence::SelectAll))
        {
            // Force start selection
            modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);
            selectAll();
        }
        else
        {
            consumed = false;
        }
    }

    if(consumed)
    {
        e->accept();
        return;
    }

    QGraphicsScene::keyReleaseEvent(e);
}

void PanelScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    QGraphicsScene::mousePressEvent(e);
}

void PanelScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    const QRectF sr = sceneRect();

    // Actual scene background
    QRectF bgRect = rect.intersected(sr);

    if(!sr.contains(rect))
    {
        // Dark gray backgound outside of scene rect
        painter->fillRect(rect, Qt::darkGray);
    }

    // Scene white background
    painter->fillRect(bgRect, Qt::white);

    if(panelsModel()->modeMgr()->mode() == FileMode::Editing)
    {
        // Draw grid lines in gray
        QPen gridPen(Qt::gray, 2.0);
        painter->setPen(gridPen);

        const int ARR_SIZE = 100;
        QLineF arr[ARR_SIZE];

        const qreal x1 = bgRect.left();
        const qreal x2 = bgRect.right();
        const qreal t  = bgRect.top();
        const qreal b  = bgRect.bottom();

        const int minH = qCeil(sr.top() / TileLocation::Size);
        const int maxH  = qFloor(sr.bottom() / TileLocation::Size);
        const int minV = qCeil(sr.left() / TileLocation::Size);
        const int maxV  = qFloor(sr.right() / TileLocation::Size);

        if(minH > maxH || minV > maxV)
            return; // Nothing to draw

        int firstH = qCeil(t / TileLocation::Size) - 1;
        int lastH  = qFloor(b / TileLocation::Size) + 1;
        int firstV = qCeil(x1 / TileLocation::Size) - 1;
        int lastV  = qFloor(x2 / TileLocation::Size) + 1;

        firstH = qBound(minH, firstH, maxH);
        lastH = qBound(minH, lastH, maxH);
        firstV = qBound(minV, firstV, maxV);
        lastV = qBound(minV, lastV, maxV);

        const int horizN = lastH - firstH;
        if (horizN > 0)
        {
            qreal y = firstH * TileLocation::Size;
            int count = 0;
            while(count < horizN)
            {
                int i = 0;
                for (; i < ARR_SIZE; i++)
                {
                    arr[i] = QLineF(x1, y, x2, y);
                    y += TileLocation::Size;

                    if(count >= horizN)
                        break;
                    count++;
                }
                painter->drawLines(arr, i + 1);
            }
        }

        const int vertN = lastV - firstV;
        if (vertN > 0)
        {
            qreal x = firstV * TileLocation::Size;
            int count = 0;
            while(count < vertN)
            {
                int i = 0;
                for (; i < ARR_SIZE; i++)
                {
                    arr[i] = QLineF(x, t, x, b);
                    x += TileLocation::Size;

                    if(count >= vertN)
                        break;
                    count++;
                }
                painter->drawLines(arr, i + 1);
            }
        }
    }
}

PanelListModel *PanelScene::panelsModel() const
{
    return static_cast<PanelListModel *>(parent());
}
