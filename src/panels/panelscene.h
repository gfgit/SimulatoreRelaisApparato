/**
 * src/panels/panelscene.h
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

#ifndef PANELSCENE_H
#define PANELSCENE_H

#include <QGraphicsScene>

#include <unordered_map>

#include "../utils/tilerotate.h"

#include "../enums/filemodes.h"

class AbstractPanelItem;

class QJsonObject;
class QJsonArray;
class PanelItemFactory;

class PanelListModel;

class ModeManager;

class PanelScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit PanelScene(PanelListModel *parent);
    ~PanelScene();

    FileMode mode() const;
    void setMode(FileMode newMode, FileMode oldMode);

    void addNode(AbstractPanelItem *item);
    void removeNode(AbstractPanelItem *item);

    AbstractPanelItem *getNodeAt(TileLocation l) const;

    void removeAllItems();
    bool loadFromJSON(const QJsonObject &obj, PanelItemFactory *factory);
    void saveToJSON(QJsonObject &obj) const;

    bool hasUnsavedChanges() const;
    void setHasUnsavedChanges(bool newHasUnsavedChanged);

    QString panelName() const;
    bool setPanelName(const QString &newCircuitSheetName);

    QString panelLongName() const;
    void setPanelLongName(const QString &newLongName);

    PanelListModel *panelsModel() const;

    ModeManager *modeMgr() const;

    void copySelectedItems();
    bool tryPasteItems(const QPointF &tileHint,
                       QPointF &outTopLeft,
                       QPointF &outBottomRight);

    void removeSelectedItems();
    void selectAll();
    void invertSelection();

signals:
    void nameChanged(const QString& newName, PanelScene *self);
    void longNameChanged(const QString& newName, PanelScene *self);

    void sceneEdited(bool val);

protected:
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    friend class AbstractPanelItem;

    bool updateItemLocation(AbstractPanelItem *item);

    AbstractPanelItem *itemBeingMoved() const;
    void startMovingItem(AbstractPanelItem *item);
    void endMovingItem();

    void requestEditNode(AbstractPanelItem *item);

    friend class PanelListModel;
    void onEditingSubModeChanged(EditingSubMode oldMode, EditingSubMode newMode);

    void startItemSelection();
    void endItemSelection();
    void allowItemSelection(bool enabled);

    void onItemSelected(AbstractPanelItem *item, bool value);

    void moveSelectionBy(int16_t dx, int16_t dy);
    void endSelectionMove();

    static constexpr QLatin1String PanelMimeType = QLatin1String("application/x-simulatore-rele-panels");

    bool insertFragment(const QPointF &tileHint,
                        const QJsonObject& fragmentRoot,
                        PanelItemFactory *factory,
                        QPointF &outTopLeft,
                        QPointF &outBottomRight);

    struct FragmentData
    {
        QVector<QJsonObject> validNodes;

        bool topLeftSet = false;
        bool bottomRightSet = false;
        QPointF topLeftLocation;
        QPointF bottomRightLocation;

        inline void trackFragmentBounds(const QPointF& pos)
        {
            if(!topLeftSet)
            {
                topLeftSet = true;
                topLeftLocation = pos;
            }
            else
            {
                if(pos.x() < topLeftLocation.x())
                    topLeftLocation.rx() = pos.x();
                if(pos.y() < topLeftLocation.y())
                    topLeftLocation.ry() = pos.y();
            }

            if(!bottomRightSet)
            {
                bottomRightSet = true;
                bottomRightLocation = pos;
            }
            else
            {
                if(pos.x() > bottomRightLocation.x())
                    bottomRightLocation.rx() = pos.x();
                if(pos.y() > bottomRightLocation.y())
                    bottomRightLocation.ry() = pos.y();
            }
        }
    };

    bool checkFragment(const QJsonArray& nodes, const QJsonArray& cables,
                       FragmentData &fragment);

private:
    QString mCircuitSheetName;
    QString mCircuitSheetLongName;

    std::unordered_map<TileLocation, AbstractPanelItem *, TileLocationHash> mItemMap;

    AbstractPanelItem *mItemBeingMoved = nullptr;

    bool m_hasUnsavedChanges = false;

    std::unordered_map<AbstractPanelItem *, TileLocation> mSelectedItemPositions;
};

#endif // PANELSCENE_H
