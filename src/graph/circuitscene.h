/**
 * src/graph/circuitscene.h
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

class AbstractCircuitNode;
class AbstractNodeGraphItem;
class PowerSourceGraphItem;
class Connector;
class CircuitCable;
class CableGraphItem;
class CableGraphPath;

class QGraphicsPathItem;

class RelaisModel;

class QJsonObject;
class NodeEditFactory;

class CircuitScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum class Mode
    {
        Static = 0,
        Simulation,
        Editing,
        LoadingFile
    };

    // On a tile there can be 2 cables if they go in squared directions
    // See CableGraphPath::addTile()
    typedef std::pair<CableGraphItem *, CableGraphItem*> TileCablePair;


    explicit CircuitScene(QObject *parent = nullptr);
    ~CircuitScene();

    Mode mode() const;
    void setMode(Mode newMode);

    void addNode(AbstractNodeGraphItem *item);
    void removeNode(AbstractNodeGraphItem *item);

    void addCable(CableGraphItem *item);
    void removeCable(CircuitCable *cable);
    CableGraphItem *graphForCable(CircuitCable *cable) const;

    TileLocation getNewFreeLocation();

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

    RelaisModel *relaisModel() const;

    void setRelaisModel(RelaisModel *newRelaisModel);

    void removeAllItems();
    bool loadFromJSON(const QJsonObject &obj, NodeEditFactory *factory);
    void saveToJSON(QJsonObject &obj) const;

signals:
    void modeChanged(Mode mode);
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

protected:
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

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

    void stopUnfinishedOperations();

    void requestEditNode(AbstractNodeGraphItem *item);
    void requestEditCable(CableGraphItem *item);

private:
    Mode mMode = Mode::Editing;

    std::unordered_map<TileLocation, AbstractNodeGraphItem *, TileLocationHash> mItemMap;

    QVector<PowerSourceGraphItem *> mPowerSources;

    std::unordered_map<CircuitCable *, CableGraphItem *> mCables;

    std::unordered_map<TileLocation, TileCablePair, TileLocationHash> mCableTiles;

    bool mIsEditingNewCable = false;
    CableGraphItem *mEditingCable = nullptr;
    QGraphicsPathItem *mEditOverlay = nullptr;
    QGraphicsPathItem *mEditNewPath = nullptr;
    CableGraphPath *mEditNewCablePath = nullptr;

    AbstractNodeGraphItem *mItemBeingMoved = nullptr;
    TileLocation mLastMovedItemValidLocation = TileLocation::invalid;

    RelaisModel *mRelaisModel = nullptr;
};

#endif // CIRCUITSCENE_H
