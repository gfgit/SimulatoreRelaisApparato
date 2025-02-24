/**
 * src/circuits/circuitscene.h
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

#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>

#include <unordered_map>

#include "../utils/tilerotate.h"

#include "../enums/filemodes.h"

class AbstractCircuitNode;
class AbstractNodeGraphItem;
class Connector;
class CircuitCable;
class CableGraphItem;
class CableGraphPath;

class QGraphicsPathItem;

class QJsonObject;
class QJsonArray;
class NodeEditFactory;

class CircuitListModel;

class ModeManager;

class CircuitScene : public QGraphicsScene
{
    Q_OBJECT
public:
    // On a tile there can be 2 cables if they go in squared directions
    // See CableGraphPath::addTile()
    typedef std::pair<CableGraphItem *, CableGraphItem*> TileCablePair;
    typedef std::pair<std::optional<CableGraphPath>, std::optional<CableGraphPath>> TileCablePathPair;
    typedef std::unordered_map<TileLocation, TileCablePair, TileLocationHash> CablePairMap;


    explicit CircuitScene(CircuitListModel *parent);
    ~CircuitScene();

    inline FileMode mode() const { return mMode; }
    void setMode(FileMode newMode, FileMode oldMode);

    void addNode(AbstractNodeGraphItem *item);
    void removeNode(AbstractNodeGraphItem *item);

    void addCable(CableGraphItem *item);
    void removeCable(CircuitCable *cable);
    CableGraphItem *graphForCable(CircuitCable *cable) const;

    TileLocation getFreeLocationNear(TileLocation origin = TileLocation::invalid);

    static QPointF getConnectorPoint(TileLocation l, Connector::Direction direction);

    void startEditNewCable();
    void startEditCable(CableGraphItem *item);
    void endEditCable(bool apply = true);

    inline bool isEditingCable() const { return mEditingCable || mIsEditingNewCable; }

    void editCableAddPoint(const QPointF& p, bool allowEdge);
    void editCableUndoLast();

    bool isLocationFree(TileLocation l) const;
    AbstractNodeGraphItem *getNodeAt(TileLocation l) const;
    TileCablePair getCablesAt(TileLocation l) const;

    bool cablePathIsValid(const CableGraphPath& cablePath, CableGraphItem *item) const;

    void removeAllItems();
    bool loadFromJSON(const QJsonObject &obj, NodeEditFactory *factory);
    void saveToJSON(QJsonObject &obj) const;

    bool hasUnsavedChanges() const;
    void setHasUnsavedChanges(bool newHasUnsavedChanged);

    QString circuitSheetName() const;
    bool setCircuitSheetName(const QString &newCircuitSheetName);

    QString circuitSheetLongName() const;
    void setCircuitSheetLongName(const QString &newLongName);

    CircuitListModel *circuitsModel() const;

    bool splitCableAt(CableGraphItem *item, const TileLocation &splitLoc);

    ModeManager *modeMgr() const;

    void copySelectedItems();
    bool tryPasteItems(const TileLocation& tileHint,
                       TileLocation &outTopLeft,
                       TileLocation &outBottomRight);

    void removeSelectedItems();
    void selectAll();
    void invertSelection();

    static Connector::Direction getTileAndDirection(const QPointF& pos,
                                                    TileLocation &outLocation, bool &isEdge);

    AbstractNodeGraphItem *getGraphForNode(AbstractCircuitNode *node) const;

signals:
    void nameChanged(const QString& newName, CircuitScene *self);
    void longNameChanged(const QString& newName, CircuitScene *self);

    void sceneEdited(bool val);

protected:
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    friend class AbstractNodeGraphItem;

    void refreshItemConnections(AbstractNodeGraphItem *item, bool tryReconnect);

    bool updateItemLocation(TileLocation newLocation,
                            AbstractNodeGraphItem *item);

    void calculateConnections();
    void connectItems(AbstractCircuitNode *node1,
                      AbstractCircuitNode *node2,
                      const Connector& c1, const Connector& c2,
                      QVector<CircuitCable *>& verifiedCables);

    bool checkCable(CableGraphItem *item);
    void checkItem(AbstractNodeGraphItem *item, QVector<CircuitCable *> &verifiedCables);

    inline AbstractNodeGraphItem *getItemAt(TileLocation l) const
    {
        auto it = mItemMap.find(l);
        if(it != mItemMap.cend())
            return it->second;
        return nullptr;
    }

    friend class CableGraphItem;
    void addCableTiles(CableGraphItem *item);
    void removeCableTiles(CableGraphItem *item);
    void editCableUpdatePen();

    AbstractNodeGraphItem *itemBeingMoved() const;
    void startMovingItem(AbstractNodeGraphItem *item);
    void endMovingItem();

    void requestEditNode(AbstractNodeGraphItem *item);
    void requestEditCable(CableGraphItem *item);

    friend class CircuitListModel;
    void onEditingSubModeChanged(EditingSubMode oldMode, EditingSubMode newMode);

    void startItemSelection();
    void endItemSelection();
    void allowItemSelection(bool enabled);

    void onItemSelected(AbstractNodeGraphItem *item, bool value);
    void onCableSelected(CableGraphItem *item, bool value);

    void moveSelectionBy(int16_t dx, int16_t dy);
    void endSelectionMove();

    void moveSelectedCableAt(const TileLocation& tile);

    static constexpr QLatin1String CircuitMimeType = QLatin1String("application/x-simulatore-rele-circuits");

    bool insertFragment(const TileLocation& tileHint,
                        const QJsonObject& fragmentRoot,
                        NodeEditFactory *factory,
                        TileLocation &outTopLeft,
                        TileLocation &outBottomRight);

    struct FragmentData
    {
        QVector<QJsonObject> validNodes;
        QVector<TileLocation> pastedNodeTiles;

        QVector<QJsonObject> validCables;
        QVector<CableGraphPath> cablePathVec;

        TileLocation topLeftLocation = TileLocation::invalid;
        TileLocation bottomRightLocation = TileLocation::invalid;

        inline void trackFragmentBounds(const TileLocation& tile)
        {
            if(!topLeftLocation.isValid())
                topLeftLocation = tile;
            else
            {
                if(tile.x < topLeftLocation.x)
                    topLeftLocation.x = tile.x;
                if(tile.y < topLeftLocation.y)
                    topLeftLocation.y = tile.y;
            }

            if(!bottomRightLocation.isValid())
                bottomRightLocation = tile;
            else
            {
                if(tile.x > bottomRightLocation.x)
                    bottomRightLocation.x = tile.x;
                if(tile.y > bottomRightLocation.y)
                    bottomRightLocation.y = tile.y;
            }
        }
    };

    bool checkFragment(const QJsonArray& nodes, const QJsonArray& cables,
                       FragmentData &fragment);

    bool locateSpaceForFragment(const FragmentData &fragment,
                                const TileLocation& tileHint,
                                TileLocation& result);

private:
    QString mCircuitSheetName;
    QString mCircuitSheetLongName;

    std::unordered_map<TileLocation, AbstractNodeGraphItem *, TileLocationHash> mItemMap;

    QVector<AbstractCircuitNode *> mPowerSources;

    std::unordered_map<CircuitCable *, CableGraphItem *> mCables;

    CablePairMap mCableTiles;

    bool mIsEditingNewCable = false;
    CableGraphItem *mEditingCable = nullptr;
    QGraphicsPathItem *mEditOverlay = nullptr;
    QGraphicsPathItem *mEditNewPath = nullptr;
    CableGraphPath *mEditNewCablePath = nullptr;

    AbstractNodeGraphItem *mItemBeingMoved = nullptr;
    TileLocation mLastMovedItemValidLocation = TileLocation::invalid;
    TileLocation mMovedItemOriginalLocation = TileLocation::invalid;

    bool m_hasUnsavedChanges = false;

    std::unordered_map<AbstractNodeGraphItem *, TileLocation> mSelectedItemPositions;
    std::unordered_map<CableGraphItem *, std::pair<TileLocation, TileLocation>> mSelectedCablePositions;
    TileLocation mSelectedCableMoveStart = TileLocation::invalid;

    FileMode mMode = FileMode::Editing;
};

#endif // CIRCUITSCENE_H
