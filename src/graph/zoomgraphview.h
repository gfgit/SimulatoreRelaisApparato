/**
 * src/graph/zoomgraphview.h
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

#ifndef ZOOMGRAPHVIEW_H
#define ZOOMGRAPHVIEW_H

#include <QGraphicsView>

class ZoomGraphView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomGraphView(QWidget *parent = nullptr);

signals:
    void zoomChanged(double val);

public slots:
    void setZoom(double val);

protected:
    bool viewportEvent(QEvent *e) override;

private:
    void doZoomBy(double factor);

private:
    static constexpr Qt::KeyboardModifier ZoomModifiers = Qt::ControlModifier;
    static constexpr double ZoomFactorBase = 1.0015;

    double mZoomFactor = 1.0;

    QPointF targetScenePos, targetViewportPos;
};

#endif // ZOOMGRAPHVIEW_H
