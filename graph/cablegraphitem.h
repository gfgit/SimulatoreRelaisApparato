#ifndef CABLEGRAPHITEM_H
#define CABLEGRAPHITEM_H

#include <QGraphicsObject>
#include <QPainterPath>
#include <QPen>

#include "../tilerotate.h"

class CircuitCable;

class CableGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    CableGraphItem(CircuitCable *cable_);

    QPainterPath path() const;
    void setPath(const QPainterPath &path);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    CircuitCable *cable() const;

    bool cableZeroLength() const;

    TileLocation sideA() const;

    TileLocation sideB() const;

    Connector::Direction directionA() const;

    Connector::Direction directionB() const;

signals:
    void editRequested(CableGraphItem *self);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

private slots:
    void updatePen();
    void triggerUpdate();

private:
    CircuitCable *mCable;
    QPen pen;
    QPainterPath mPath;
    QRectF mBoundingRect;
    QRectF mUnconnectedRectA;
    QRectF mUnconnectedRectB;

    TileLocation mSideA = TileLocation::invalid;
    Connector::Direction mDirectionA = Connector::Direction::North;

    TileLocation mSideB = TileLocation::invalid;
    Connector::Direction mDirectionB = Connector::Direction::South;
    bool mCableZeroLength = true;
};

#endif // CABLEGRAPHITEM_H
