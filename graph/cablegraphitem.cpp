#include "cablegraphitem.h"

#include "../circuitcable.h"

#include <QPainterPathStroker>
#include <QPainter>

static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);
    if (path == QPainterPath() || pen == Qt::NoPen)
        return path;
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    if (pen.widthF() <= 0.0)
        ps.setWidth(penWidthZero);
    else
        ps.setWidth(pen.widthF());
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}

CableGraphItem::CableGraphItem(CircuitCable *cable)
    : QGraphicsObject()
    , mCable(cable)
{
    setParent(mCable);

    mPath.moveTo(0, 1);
    mPath.lineTo(0, 49);

    pen.setColor(Qt::black);
    pen.setWidthF(5.0);

    connect(mCable, &CircuitCable::modeChanged, this, &CableGraphItem::updatePen);
    connect(mCable, &CircuitCable::powerChanged, this, &CableGraphItem::updatePen);

    updatePen();
}

QRectF CableGraphItem::boundingRect() const
{
    if (mBoundingRect.isNull())
    {
        CableGraphItem *self = const_cast<CableGraphItem*>(this);
        qreal pw = pen.style() == Qt::NoPen ? qreal(0) : pen.widthF();
        if (pw == 0.0)
            self->mBoundingRect = mPath.controlPointRect();
        else
            self->mBoundingRect = shape().controlPointRect();
    }
    return mBoundingRect;
}

QPainterPath CableGraphItem::shape() const
{
    return qt_graphicsItem_shapeFromPath(mPath, pen);
}

void CableGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(mPath);
}

void CableGraphItem::setPath(const QPainterPath &path)
{
    if (mPath == path)
        return;
    prepareGeometryChange();
    mPath = path;
    mBoundingRect = QRectF();
    update();
}

void CableGraphItem::updatePen()
{
    prepareGeometryChange();
    mBoundingRect = QRectF();

    const auto power = mCable->powered();

    Qt::GlobalColor color = Qt::black;
    if(power != CircuitCable::Power::None)
        color = Qt::red;

    Qt::PenStyle style = Qt::SolidLine;
    if(power != CircuitCable::Power::None &&
            power != CircuitCable::Power::BothOn &&
            mCable->mode() == CircuitCable::Mode::Bifilar)
        style = Qt::DashLine;

    pen.setColor(color);
    pen.setStyle(style);

    update();
}
