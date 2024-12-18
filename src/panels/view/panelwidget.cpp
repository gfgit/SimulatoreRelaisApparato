/**
 * src/panels/view/panelwidget.cpp
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

#include "panelwidget.h"
#include "panelview.h"

#include "../panelscene.h"
#include "../edit/panelitemfactory.h"

#include "../abstractpanelitem.h"

#include "../../views/viewmanager.h"

#include "../../utils/doubleclickslider.h"
#include <QDoubleSpinBox>

#include <QVBoxLayout>

#include <QKeyEvent>

#include <QInputDialog>

PanelWidget::PanelWidget(ViewManager *mgr, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(mgr)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(QMargins());
    setContentsMargins(QMargins());

    mCircuitView = new PanelView;
    mCircuitView->installEventFilter(this);
    lay->addWidget(mCircuitView);

    statusBar = new QWidget;
    statusBar->installEventFilter(this);
    statusBar->setContentsMargins(QMargins());
    statusBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    QHBoxLayout *statusLay = new QHBoxLayout(statusBar);
    statusLay->setContentsMargins(QMargins());
    statusLay->addStretch();

    mZoomSlider = new DoubleClickSlider(Qt::Horizontal);
    mZoomSlider->installEventFilter(this);
    mZoomSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mZoomSlider->setRange(0, ZoomGraphView::MaxZoom * 100);
    mZoomSlider->setTickInterval(25);
    mZoomSlider->setTickPosition(QSlider::TicksBothSides);
    statusLay->addWidget(mZoomSlider);

    mZoomSpin = new QDoubleSpinBox;
    mZoomSpin->installEventFilter(this);
    mZoomSpin->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mZoomSpin->setRange(ZoomGraphView::MinZoom * 100,
                        ZoomGraphView::MaxZoom * 100);
    statusLay->addWidget(mZoomSpin);

    lay->addWidget(statusBar);

    connect(mCircuitView, &ZoomGraphView::zoomChanged,
            this, &PanelWidget::onZoomChanged);
    connect(mZoomSlider, &QSlider::valueChanged,
            this, &PanelWidget::onZoomSliderChanged);
    connect(mZoomSlider, &DoubleClickSlider::sliderHandleDoubleClicked,
            this, &PanelWidget::resetZoom);
    connect(mZoomSpin, &QDoubleSpinBox::valueChanged,
            this, &PanelWidget::onZoomSpinChanged);

    onZoomChanged(mCircuitView->zoomFactor());
}

PanelWidget::~PanelWidget()
{

}

PanelScene *PanelWidget::scene() const
{
    return mScene;
}

void PanelWidget::setScene(PanelScene *newScene, bool updateName)
{
    if(mScene == newScene)
        return;

    if(mScene)
    {
        disconnect(mScene, &PanelScene::nameChanged,
                   this, &PanelWidget::onSceneNameChanged);
        disconnect(mScene, &PanelScene::destroyed,
                   this, &PanelWidget::onSceneDestroyed);
    }

    mScene = newScene;
    mCircuitView->setScene(mScene);

    if(mScene)
    {
        connect(mScene, &PanelScene::nameChanged,
                this, &PanelWidget::onSceneNameChanged);
        connect(mScene, &PanelScene::destroyed,
                this, &PanelWidget::onSceneDestroyed);
    }

    setUniqueNum(mViewMgr->getUniqueNum(mScene, this));

    if(updateName)
        onSceneNameChanged();
}

void PanelWidget::onZoomChanged(double val)
{
    QSignalBlocker blk(mZoomSpin);
    mZoomSpin->setValue(val * 100.0);

    QSignalBlocker blk2(mZoomSlider);
    mZoomSlider->setValue(qRound(mZoomSpin->value()));
}

void PanelWidget::onZoomSliderChanged(int val)
{
    mCircuitView->setZoom(double(val) / 100.0);
}

void PanelWidget::onZoomSpinChanged(double val)
{
    mCircuitView->setZoom(val / 100.0);
}

void PanelWidget::resetZoom()
{
    mCircuitView->setZoom(1.0);
}

void PanelWidget::onSceneNameChanged()
{
    mViewMgr->updateDockName(this);
}

void PanelWidget::onSceneDestroyed()
{
    setScene(nullptr);
}

bool PanelWidget::eventFilter(QObject *watched, QEvent *e)
{
    if(watched == mCircuitView ||
            watched == mZoomSlider ||
            watched == mZoomSpin ||
            watched == statusBar)
    {
        if(e->type() == QEvent::FocusIn)
        {
            // Set this view as active
            mViewMgr->setActivePanel(this);
        }
        else if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent *ev = static_cast<QKeyEvent *>(e);
            if(ev->modifiers() == Qt::ShiftModifier
                    && ev->key() == Qt::Key_Z)
            {
                toggleStatusBar();
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, e);
}

void PanelWidget::focusInEvent(QFocusEvent *ev)
{
    // Set this view as active
    mViewMgr->setActivePanel(this);

    QWidget::focusInEvent(ev);
}

void PanelWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->modifiers() == Qt::ShiftModifier
            && ev->key() == Qt::Key_Z)
    {
        toggleStatusBar();
        return;
    }

    QWidget::keyPressEvent(ev);
}

void PanelWidget::addNodeToCenter(PanelItemFactory *editFactory,
                                  const QString &nodeType)
{
    if(!mScene)
        return;

    const auto needsName = editFactory->needsName(nodeType);
    QString name;
    if(needsName == PanelItemFactory::NeedsName::Always)
    {
        name = QInputDialog::getText(this,
                                     tr("Choose Item Name"),
                                     tr("Name:"));
        if(name.isEmpty())
            return;
    }

    QPoint vpCenter = mCircuitView->viewport()->rect().center();
    QPointF sceneCenter = mCircuitView->mapToScene(vpCenter);

    auto item = editFactory->createItem(nodeType, mScene);

    if(needsName == PanelItemFactory::NeedsName::Always)
        item->setObjectName(name);

    item->setPos(sceneCenter);

    // Add node to scene
    scene()->addNode(item);

    mCircuitView->ensureVisible(item);
}

void PanelWidget::toggleStatusBar()
{
    statusBar->setVisible(!statusBar->isVisible());
}

int PanelWidget::uniqueNum() const
{
    return mUniqueNum;
}

void PanelWidget::setUniqueNum(int newUniqueNum)
{
    mUniqueNum = newUniqueNum;
}

PanelView *PanelWidget::panelView() const
{
    return mCircuitView;
}
