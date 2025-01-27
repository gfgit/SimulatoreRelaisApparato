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

#include "graphs/lightrectitem.h"

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

    const bool editing = newMode == FileMode::Editing;

    for(LightRectItem *item : mLightRects)
        item->setZValue(editing ?
                            int(Layers::EditingLightRects) :
                            int(Layers::LightRects));

    if(editing)
    {
        setSceneRect(QRectF());

        buildSnapMap();
    }
    else
    {
        if(oldMode == FileMode::Editing || oldMode == FileMode::LoadingFile)
        {
            // Cut scene rect to items bounding rect
            QRectF br = itemsBoundingRect();

            // Add some margin
            br.adjust(-TileLocation::HalfSize, -TileLocation::HalfSize,
                      TileLocation::HalfSize, TileLocation::HalfSize);
            setSceneRect(br);
        }

        clearSnapMap();

        bringTop(nullptr); // Reset topmost item
    }

    allowItemSelection(editing);

    // TODO: not really...
    // Background changes between modes
    invalidate(QRectF(), BackgroundLayer);
}

void PanelScene::addNode(AbstractPanelItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    const bool editing = modeMgr()->mode() == FileMode::Editing;

    // Add item after having inserted it in the map
    addItem(item);

    if(item->itemType() == LightRectItem::ItemType)
    {
        LightRectItem *lightItem = static_cast<LightRectItem *>(item);
        mLightRects.append(lightItem);
        if(editing)
        {
            lightItem->setZValue(int(Layers::EditingLightRects));
        }
        else
            lightItem->setZValue(int(Layers::LightRects));
    }
    else
    {
        mOtherPanelItems.append(item);
        item->setZValue(int(Layers::OtherItems));

        if(editing)
            registerSnap(item->boundingRect().translated(item->pos()));
    }

    item->setFlag(QGraphicsItem::ItemIsSelectable, editing);
    item->setFlag(QGraphicsItem::ItemIsMovable, editing);

    setHasUnsavedChanges(true);
}

void PanelScene::removeNode(AbstractPanelItem *item)
{
    const bool editing = modeMgr()->mode() == FileMode::Editing;

    if(item->itemType() == LightRectItem::ItemType)
    {
        if(mTopLightRect == item)
            bringTop(nullptr);
        mLightRects.removeOne(static_cast<LightRectItem *>(item));
    }
    else
    {
        mOtherPanelItems.removeOne(item);

        if(editing)
            unregisterSnap(item->boundingRect().translated(item->pos()));
    }

    removeItem(item);

    setHasUnsavedChanges(true);
}

bool PanelScene::updateItemLocation(AbstractPanelItem *item)
{
    setHasUnsavedChanges(true);
    return true;
}

void PanelScene::requestEditNode(AbstractPanelItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    if(mode() == FileMode::Editing)
        emit panelsModel()->itemEditRequested(item);
}

void PanelScene::onEditingSubModeChanged(EditingSubMode oldMode, EditingSubMode newMode)
{

}

void PanelScene::allowItemSelection(bool enabled)
{
    for(AbstractPanelItem* item : mOtherPanelItems)
    {
        item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        item->setFlag(QGraphicsItem::ItemIsMovable, enabled);
    }

    for(AbstractPanelItem* item : mLightRects)
    {
        item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        item->setFlag(QGraphicsItem::ItemIsMovable, enabled);
    }
}

void PanelScene::onItemSelected(AbstractPanelItem *item, bool value)
{
    if(value)
    {
        if(item->itemType() == LightRectItem::ItemType)
            bringTop(static_cast<LightRectItem *>(item));
    }
    else
    {
        if(mTopLightRect == item)
            bringTop(nullptr);
    }
}

void PanelScene::bringTop(LightRectItem *item)
{
    if(item == mTopLightRect)
        return; // Already at top

    if(mTopLightRect)
        mTopLightRect->setZValue(int(Layers::EditingLightRects));

    mTopLightRect = item;

    if(mTopLightRect)
        mTopLightRect->setZValue(int(Layers::TopMostLightRect));
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

    for(AbstractPanelItem* item : mOtherPanelItems)
    {
        item->setSelected(true);
    }

    for(AbstractPanelItem* item : mLightRects)
    {
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
    clearSelection();

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
            if(!item->loadFromJSON(obj, modeMgr()))
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

void PanelScene::buildSnapMap()
{
    clearSnapMap();

    for(AbstractPanelItem* item : mOtherPanelItems)
    {
        registerSnap(item->boundingRect().translated(item->pos()));
    }
}

void PanelScene::clearSnapMap()
{
    mXSnapMap.clear();
    mYSnapMap.clear();
}

void PanelScene::registerSnap(const QRectF &r)
{
    auto it = mXSnapMap.find(r.left());
    if(it == mXSnapMap.end())
        it = mXSnapMap.insert(r.left(), 0);
    (*it)++;

    it = mXSnapMap.find(r.right());
    if(it == mXSnapMap.end())
        it = mXSnapMap.insert(r.right(), 0);
    (*it)++;

    it = mYSnapMap.find(r.top());
    if(it == mYSnapMap.end())
        it = mYSnapMap.insert(r.top(), 0);
    (*it)++;

    it = mYSnapMap.find(r.bottom());
    if(it == mYSnapMap.end())
        it = mYSnapMap.insert(r.bottom(), 0);
    (*it)++;
}

void PanelScene::unregisterSnap(const QRectF &r)
{
    auto it = mXSnapMap.find(r.left());
    if(it != mXSnapMap.end())
    {
        int &ref = *it;
        if(--ref == 0)
            mXSnapMap.erase(it);
    }

    it = mXSnapMap.find(r.right());
    if(it != mXSnapMap.end())
    {
        int &ref = *it;
        if(--ref == 0)
            mXSnapMap.erase(it);
    }

    it = mYSnapMap.find(r.top());
    if(it != mYSnapMap.end())
    {
        int &ref = *it;
        if(--ref == 0)
            mYSnapMap.erase(it);
    }

    it = mYSnapMap.find(r.bottom());
    if(it != mYSnapMap.end())
    {
        int &ref = *it;
        if(--ref == 0)
            mYSnapMap.erase(it);
    }
}

template <typename Map, typename T>
typename Map::const_iterator maxLowerThan(const Map& map, const T& val)
{
    // lowerBound() returns first item not-less than val
    // so we need the item previous to it
    typename Map::const_iterator it = map.lowerBound(val);

    if(it != map.cend() && it.key() == val)
        return it;

    if(it == map.cbegin())
    {
        // No previous item, return end
        return map.cend();
    }

    // Return previous item
    return --it;
}

PanelScene::SnapResult PanelScene::getSnapFor(const QPointF &target)
{
    SnapResult result;
    result.pt = target;

    typedef QMap<double, int>::const_iterator Iter;

    Iter x1 = maxLowerThan(mXSnapMap, target.x());
    Iter x2 = mXSnapMap.upperBound(target.x());

    const double deltaX1 = x1 != mXSnapMap.cend() ? (target.x() - x1.key()) : 0;
    const double deltaX2 = x2 != mXSnapMap.cend() ? (x2.key() - target.x()) : 0;

    if(x1 != mXSnapMap.cend() && (x2 == mXSnapMap.cend() || deltaX1 <= deltaX2))
    {
        if(deltaX1 < MAX_SNAP_DISTANCE)
        {
            result.pt.setX(x1.key());
            result.foundX = true;
        }
    }
    else if(x2 != mXSnapMap.cend() && (x1 == mXSnapMap.cend() || deltaX2 <= deltaX1))
    {
        if(deltaX2 < MAX_SNAP_DISTANCE)
        {
            result.pt.setX(x2.key());
            result.foundX = true;
        }
    }


    Iter y1 = maxLowerThan(mYSnapMap, target.y());
    Iter y2 = mYSnapMap.upperBound(target.y());

    const double deltaY1 = y1 != mYSnapMap.cend() ? (target.y() - y1.key()) : 0;
    const double deltaY2 = y2 != mYSnapMap.cend() ? (y2.key() - target.y()) : 0;

    if(y1 != mYSnapMap.cend() && (y2 == mYSnapMap.cend() || deltaY1 <= deltaY2))
    {
        if(deltaY1 < MAX_SNAP_DISTANCE)
        {
            result.pt.setY(y1.key());
            result.foundY = true;
        }
    }
    else if(y2 != mYSnapMap.cend() && (y1 == mYSnapMap.cend() || deltaY2 <= deltaY1))
    {
        if(deltaY2 < MAX_SNAP_DISTANCE)
        {
            result.pt.setY(y2.key());
            result.foundY = true;
        }
    }

    return result;
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

    const auto lightsCopy = mLightRects;
    for(AbstractPanelItem* item : lightsCopy)
    {
        removeNode(item);
    }
    Q_ASSERT(mLightRects.size() == 0);

    const auto itemsCopy = mOtherPanelItems;
    for(AbstractPanelItem* item : itemsCopy)
    {
        removeNode(item);
    }
    Q_ASSERT(mOtherPanelItems.size() == 0);
}

bool PanelScene::loadFromJSON(const QJsonObject &obj, PanelItemFactory *factory)
{
    removeAllItems();

    if(!obj.contains("nodes") || !obj.contains("lights"))
        return false;

    if(!setPanelName(obj.value("name").toString()))
        return false;

    setPanelLongName(obj.value("long_name").toString());

    const QJsonArray nodes = obj.value("nodes").toArray();
    const QJsonArray lights = obj.value("lights").toArray();

    auto loadItem = [this, factory](const QJsonValue& v)
    {
        const QJsonObject nodeObj = v.toObject();
        const QString nodeType = nodeObj.value("type").toString();
        if(nodeType.isEmpty())
            return;

        // Create new node
        AbstractPanelItem *item = factory->createItem(nodeType, this);
        if(item)
        {
            if(!item->loadFromJSON(nodeObj, modeMgr()))
            {
                delete item;
                return;
            }

            addNode(item);
        }
    };

    for(const QJsonValue& v : nodes)
        loadItem(v);
    for(const QJsonValue& v : lights)
        loadItem(v);

    setHasUnsavedChanges(false);

    return true;
}

void PanelScene::saveToJSON(QJsonObject &obj) const
{
    obj["name"] = panelName();
    obj["long_name"] = panelLongName();

    // Sort nodes by position
    // NOTE: since 2 items can have same position, we use stable_sort() variant
    QVector<AbstractPanelItem *> sortedItems = mOtherPanelItems;
    std::stable_sort(sortedItems.begin(),
                     sortedItems.end(),
                     [](AbstractPanelItem *a, AbstractPanelItem *b) -> bool
    {
        QPointF locA = a->pos();
        QPointF locB = b->pos();

        // Order by Y, then by X
        if(qFuzzyCompare(locA.y(), locB.y()))
            return locA.x() < locB.x();
        return locA.y() < locB.y();
    });

    QJsonArray nodes;
    for(const AbstractPanelItem *item : sortedItems)
    {
        QJsonObject nodeObj;
        item->saveToJSON(nodeObj);
        nodes.append(nodeObj);
    }
    sortedItems.clear();
    sortedItems.squeeze();

    obj["nodes"] = nodes;

    QVector<LightRectItem *> sortedLights = mLightRects;
    std::stable_sort(sortedLights.begin(),
                     sortedLights.end(),
                     [](LightRectItem *a, LightRectItem *b) -> bool
    {
        QPointF locA = a->pos();
        QPointF locB = b->pos();

        if(qFuzzyCompare(locA.y(), locB.y()))
            return locA.x() < locB.x();
        return locA.y() < locB.y();
    });

    QJsonArray lights;
    for(const AbstractPanelItem *item : sortedLights)
    {
        QJsonObject nodeObj;
        item->saveToJSON(nodeObj);
        lights.append(nodeObj);
    }
    sortedLights.clear();
    sortedLights.squeeze();

    obj["lights"] = lights;
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
    // Draw dark gray background only on scene rect

    const QRectF toBePainted = rect.intersected(sceneRect());

    painter->fillRect(toBePainted, qRgb(0x7F, 0x7F, 0x7F));
}

PanelListModel *PanelScene::panelsModel() const
{
    return static_cast<PanelListModel *>(parent());
}
