#include "cablegraphitem.h"

#include "../circuitcable.h"
#include "../abstractcircuitnode.h"

#include "circuitscene.h"

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

CableGraphItem::CableGraphItem(CircuitCable *cable_)
    : QGraphicsObject()
    , mCable(cable_)
{
    setParent(mCable);

    QRectF square(QPointF(), QSizeF(20, 20));
    mUnconnectedRectA = mUnconnectedRectB = square;

    mPath.moveTo(0, 1);
    mPath.lineTo(0, 49);

    pen.setColor(Qt::black);
    pen.setWidthF(5.0);

    connect(mCable, &CircuitCable::modeChanged, this, &CableGraphItem::updatePen);
    connect(mCable, &CircuitCable::powerChanged, this, &CableGraphItem::updatePen);
    connect(mCable, &CircuitCable::nodesChanged, this, &CableGraphItem::triggerUpdate);

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
    QPainterPath p = mPath;

    bool isAconnected = mCable->getNode(CircuitCable::Side::A).node;
    bool isBconnected = mCable->getNode(CircuitCable::Side::B).node;

    if(!isAconnected)
        p.addRect(mUnconnectedRectA);
    if(!isBconnected)
        p.addRect(mUnconnectedRectB);

    return qt_graphicsItem_shapeFromPath(p, pen);
}

void CableGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Draw square if cable end is not connected
    bool isAconnected = mCable->getNode(CircuitCable::Side::A).node;
    bool isBconnected = mCable->getNode(CircuitCable::Side::B).node;

    QColor oldColor = pen.color();
    if(!isAconnected || !isBconnected)
    {
        // Draw a cyan square on cable end
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::darkCyan);

        if(!isAconnected)
        {
            painter->drawRect(mUnconnectedRectA);
        }

        if(!isBconnected)
        {
            painter->drawRect(mUnconnectedRectB);
        }

        // Also draw line with cyan color
        pen.setColor(Qt::darkCyan);
    }

    // Draw cable path
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(mPath);

    pen.setColor(oldColor);
}

void CableGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    auto *s = qobject_cast<CircuitScene *>(scene());
    if(s && s->mode() == CircuitScene::Mode::Editing)
    {
        emit editRequested(this);
        return;
    }

    QGraphicsObject::mouseDoubleClickEvent(e);
}

QPainterPath CableGraphItem::path() const
{
    return mPath;
}

void CableGraphItem::setPath(const QPainterPath &path)
{
    if (mPath == path)
        return;
    prepareGeometryChange();
    mPath = path;
    mBoundingRect = QRectF();

    auto start = mPath.elementAt(0);
    auto end = mPath.elementAt(mPath.elementCount() - 1);

    // Calculate start end
    if(mPath.elementCount() > 1)
    {
        auto startDir = mPath.elementAt(1);
        auto endDir = mPath.elementAt(mPath.elementCount() - 2);

        if(qFuzzyCompare(start.x, startDir.x))
        {
            if(start.y < startDir.y)
                mDirectionA = Connector::Direction::North;
            else
                mDirectionA = Connector::Direction::South;
        }
        else
        {
            if(start.x < startDir.x)
                mDirectionA = Connector::Direction::West;
            else
                mDirectionA = Connector::Direction::East;
        }

        if(qFuzzyCompare(end.x, endDir.x))
        {
            if(end.y < endDir.y)
                mDirectionB = Connector::Direction::North;
            else
                mDirectionB = Connector::Direction::South;
        }
        else
        {
            if(end.x < endDir.x)
                mDirectionB = Connector::Direction::West;
            else
                mDirectionB = Connector::Direction::East;
        }

        mSideA.x = static_cast<int16_t>(std::floor(start.x / TileLocation::Size));
        mSideA.y = static_cast<int16_t>(std::floor(start.y / TileLocation::Size));
        if(mDirectionA == Connector::Direction::South)
            mSideA.y--;
        if(mDirectionA == Connector::Direction::East)
            mSideA.x--;

        mSideB.x = static_cast<int16_t>(std::floor(end.x / TileLocation::Size));
        mSideB.y = static_cast<int16_t>(std::floor(end.y / TileLocation::Size));
        if(mDirectionB == Connector::Direction::South)
            mSideB.y--;
        if(mDirectionB == Connector::Direction::East)
            mSideB.x--;

        mCableZeroLength = false;
    }
    else
    {
        mSideA.x = static_cast<int16_t>(std::floor(start.x / TileLocation::Size));
        mSideA.y = static_cast<int16_t>(std::floor(start.y / TileLocation::Size));

        if(qFuzzyCompare(start.x, mSideA.x * TileLocation::Size))
        {
            mDirectionA = Connector::Direction::West;
            mDirectionB = Connector::Direction::East;
        }
        else
        {
            mDirectionA = Connector::Direction::North;
            mDirectionB = Connector::Direction::South;
        }

        mSideB = mSideA;

        mCableZeroLength = true;
    }

    // Recalculate position of unconnected rects
    const QRectF br = mPath.controlPointRect();

    // Rect A
    if(br.center().x() > start.x)
        mUnconnectedRectA.moveLeft(start.x);
    else
        mUnconnectedRectA.moveRight(start.x);

    if(br.center().y() > start.y)
        mUnconnectedRectA.moveTop(start.y);
    else
        mUnconnectedRectA.moveBottom(start.y);

    // Rect B
    if(br.center().x() > end.x)
        mUnconnectedRectB.moveLeft(end.x);
    else
        mUnconnectedRectB.moveRight(end.x);

    if(br.center().y() > end.y)
        mUnconnectedRectB.moveTop(end.y);
    else
        mUnconnectedRectB.moveBottom(end.y);

    // Detach nodes, they will be reattached later
    CircuitCable::CableEnd cableEnd = cable()->getNode(CircuitCable::Side::A);
    if(cableEnd.node)
    {
        AbstractCircuitNode::CableItem item;
        item.cable.cable = cable();
        item.cable.side = CircuitCable::Side::A;
        item.nodeContact = cableEnd.nodeContact;

        item.cable.pole = CircuitCable::Pole::First;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != AbstractCircuitNode::ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }

        item.cable.pole = CircuitCable::Pole::Second;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != AbstractCircuitNode::ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }
    }

    cableEnd = cable()->getNode(CircuitCable::Side::B);
    if(cableEnd.node)
    {
        AbstractCircuitNode::CableItem item;
        item.cable.cable = cable();
        item.cable.side = CircuitCable::Side::B;
        item.nodeContact = cableEnd.nodeContact;

        item.cable.pole = CircuitCable::Pole::First;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != AbstractCircuitNode::ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }

        item.cable.pole = CircuitCable::Pole::Second;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != AbstractCircuitNode::ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }
    }

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
            mCable->mode() == CircuitCable::Mode::Unifilar)
        style = Qt::DashLine;

    pen.setColor(color);
    pen.setStyle(style);

    update();
}

void CableGraphItem::triggerUpdate()
{
    update();
}

Connector::Direction CableGraphItem::directionB() const
{
    return mDirectionB;
}

Connector::Direction CableGraphItem::directionA() const
{
    return mDirectionA;
}

TileLocation CableGraphItem::sideB() const
{
    return mSideB;
}

TileLocation CableGraphItem::sideA() const
{
    return mSideA;
}

bool CableGraphItem::cableZeroLength() const
{
    return mCableZeroLength;
}

CircuitCable *CableGraphItem::cable() const
{
    return mCable;
}
