#include "circuitwidget.h"

#include "../graph/circuitscene.h"
#include "../nodes/edit/nodeeditfactory.h"

#include "../graph/abstractnodegraphitem.h"
#include "../nodes/abstractcircuitnode.h"

#include "viewmanager.h"

#include "../utils/zoomgraphview.h"
#include "../utils/doubleclickslider.h"
#include <QDoubleSpinBox>

#include <QVBoxLayout>

#include <QKeyEvent>

#include <QInputDialog>

CircuitWidget::CircuitWidget(ViewManager *mgr, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(mgr)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(QMargins());
    setContentsMargins(QMargins());

    mCircuitView = new ZoomGraphView;
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
            this, &CircuitWidget::onZoomChanged);
    connect(mZoomSlider, &QSlider::valueChanged,
            this, &CircuitWidget::onZoomSliderChanged);
    connect(mZoomSlider, &DoubleClickSlider::sliderHandleDoubleClicked,
            this, &CircuitWidget::resetZoom);
    connect(mZoomSpin, &QDoubleSpinBox::valueChanged,
            this, &CircuitWidget::onZoomSpinChanged);

    onZoomChanged(mCircuitView->zoomFactor());
}

CircuitWidget::~CircuitWidget()
{

}

CircuitScene *CircuitWidget::scene() const
{
    return mScene;
}

void CircuitWidget::setScene(CircuitScene *newScene, bool updateName)
{
    if(mScene == newScene)
        return;

    if(mScene)
    {
        disconnect(mScene, &CircuitScene::nameChanged,
                   this, &CircuitWidget::onSceneNameChanged);
        disconnect(mScene, &CircuitScene::destroyed,
                   this, &CircuitWidget::onSceneDestroyed);
    }

    mScene = newScene;
    mCircuitView->setScene(mScene);

    if(mScene)
    {
        connect(mScene, &CircuitScene::nameChanged,
                this, &CircuitWidget::onSceneNameChanged);
        connect(mScene, &CircuitScene::destroyed,
                this, &CircuitWidget::onSceneDestroyed);
    }

    setUniqueNum(mViewMgr->getUniqueNum(mScene, this));

    if(updateName)
        onSceneNameChanged();
}

void CircuitWidget::onZoomChanged(double val)
{
    QSignalBlocker blk(mZoomSpin);
    mZoomSpin->setValue(val * 100.0);

    QSignalBlocker blk2(mZoomSlider);
    mZoomSlider->setValue(qRound(mZoomSpin->value()));
}

void CircuitWidget::onZoomSliderChanged(int val)
{
    mCircuitView->setZoom(double(val) / 100.0);
}

void CircuitWidget::onZoomSpinChanged(double val)
{
    mCircuitView->setZoom(val / 100.0);
}

void CircuitWidget::resetZoom()
{
    mCircuitView->setZoom(1.0);
}

void CircuitWidget::onSceneNameChanged()
{
    mViewMgr->updateDockName(this);
}

void CircuitWidget::onSceneDestroyed()
{
    setScene(nullptr);
}

bool CircuitWidget::eventFilter(QObject *watched, QEvent *e)
{
    if(watched == mCircuitView ||
            watched == mZoomSlider ||
            watched == mZoomSpin ||
            watched == statusBar)
    {
        if(e->type() == QEvent::FocusIn)
        {
            // Set this view as active
            mViewMgr->setActiveCircuit(this);
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

void CircuitWidget::focusInEvent(QFocusEvent *ev)
{
    // Set this view as active
    mViewMgr->setActiveCircuit(this);

    QWidget::focusInEvent(ev);
}

void CircuitWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->modifiers() == Qt::ShiftModifier
            && ev->key() == Qt::Key_Z)
    {
        toggleStatusBar();
        return;
    }

    QWidget::keyPressEvent(ev);
}

void CircuitWidget::addNodeToCenter(NodeEditFactory *editFactory,
                                    const QString &nodeType)
{
    if(!mScene)
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

    QPoint vpCenter = mCircuitView->viewport()->rect().center();
    QPointF sceneCenter = mCircuitView->mapToScene(vpCenter);
    TileLocation hint = TileLocation::fromPoint(sceneCenter);

    auto item = editFactory->createItem(nodeType, mScene, hint);

    if(needsName == NodeEditFactory::NeedsName::Always)
        item->getAbstractNode()->setObjectName(name);

    mCircuitView->ensureVisible(item);
}

void CircuitWidget::toggleStatusBar()
{
    statusBar->setVisible(!statusBar->isVisible());
}

int CircuitWidget::uniqueNum() const
{
    return mUniqueNum;
}

void CircuitWidget::setUniqueNum(int newUniqueNum)
{
    mUniqueNum = newUniqueNum;
}

ZoomGraphView *CircuitWidget::circuitView() const
{
    return mCircuitView;
}
