/**
 * src/circuits/view/circuitsview.h
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

#ifndef CIRCUITSVIEW_H
#define CIRCUITSVIEW_H

#include "../../utils/zoomgraphview.h"

#include "../../enums/filemodes.h"

class CircuitScene;
struct TileLocation;
class NodeEditFactory;

class AbstractNodeGraphItem;

class ViewManager;

class CircuitsView : public ZoomGraphView
{
    Q_OBJECT
public:
    explicit CircuitsView(ViewManager *viewMgr, QWidget *parent = nullptr);

    CircuitScene *circuitScene() const;

    void addNodeAtLocation(NodeEditFactory *editFactory,
                           const QString &nodeType,
                           const TileLocation& tileHint);

    void setMode(FileMode newMode, FileMode oldMode);

    void batchNodeEdit();
    void batchObjectReplace();

    inline ViewManager *viewMgr() const { return mViewMgr; }

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;

private:
    void deleteSelectedItems();

    void renderToSVG(const QString& fileName);

    void ensureItemsSelected(const QVector<AbstractNodeGraphItem *>& items);

private:
    ViewManager *mViewMgr = nullptr;
};

#endif // CIRCUITSVIEW_H
