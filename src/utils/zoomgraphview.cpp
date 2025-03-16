/**
 * src/utils/zoomgraphview.cpp
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

#include "zoomgraphview.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <QScrollBar>

ZoomGraphView::ZoomGraphView(QWidget *parent)
    : QGraphicsView{parent}
{
    setMouseTracking(true);
}

void ZoomGraphView::setZoom(double val)
{
    val = qBound(MinZoom, val, MaxZoom);

    if(qFuzzyCompare(val, mZoomFactor))
        return;

    mZoomFactor = val;

    QPointF targetScenePos = mapToScene(targetViewportPos.toPoint());

    setTransform(QTransform::fromScale(mZoomFactor, mZoomFactor));
    centerOn(targetScenePos);

    QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0,
                                                           viewport()->height() / 2.0);

    QPointF vpCenter = mapFromScene(targetScenePos) - deltaViewportPos;
    centerOn(mapToScene(vpCenter.toPoint()));

    emit zoomChanged(mZoomFactor);
}

bool ZoomGraphView::viewportEvent(QEvent *e)
{
    if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEv = static_cast<QMouseEvent*>(e);
        QPointF delta = targetViewportPos - mouseEv->pos();

        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            targetViewportPos = mouseEv->pos();
        }
    }
    else if (e->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEv = static_cast<QWheelEvent*>(e);
        if (wheelEv->modifiers() == ZoomModifiers)
        {
            double angle = wheelEv->angleDelta().y();
            double factor = qPow(ZoomFactorBase, angle);
            zoomBy(factor);

            // Eat the event
            return true;
        }
    }

    return QGraphicsView::viewportEvent(e);
}

void ZoomGraphView::mousePressEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::RightButton && e->modifiers() == Qt::AltModifier)
    {
        mIsPanning = true;
        mPanStart = e->position();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    QGraphicsView::mousePressEvent(e);
}

void ZoomGraphView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::RightButton && mIsPanning)
    {
        QPoint delta = (e->position() - mPanStart).toPoint();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        mPanStart = e->pos();
        e->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(e);
}

void ZoomGraphView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton && mIsPanning)
    {
        stopPanOperation();
        e->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(e);
}

void ZoomGraphView::zoomBy(double factor)
{
    setZoom(mZoomFactor * factor); // Accumulate
}

double ZoomGraphView::zoomFactor() const
{
    return mZoomFactor;
}

QPointF ZoomGraphView::getTargetScenePos() const
{
    return mapToScene(targetViewportPos.toPoint());
}

void ZoomGraphView::stopPanOperation()
{
    if(!mIsPanning)
        return;

    mIsPanning = false;
    setCursor(Qt::ArrowCursor);
}

QPoint ZoomGraphView::getScrollPosition() const
{
    return QPoint{horizontalScrollBar()->value(),
                  verticalScrollBar()->value()};
}

void ZoomGraphView::setScrollPosition(const QPoint &scrollPos) const
{
    horizontalScrollBar()->setValue(scrollPos.x());
    verticalScrollBar()->setValue(scrollPos.y());
}
