#ifndef CABLEGRAPHITEM_H
#define CABLEGRAPHITEM_H

#include <QGraphicsObject>
#include <QPainterPath>
#include <QPen>

class CircuitCable;

class CableGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    CableGraphItem(CircuitCable *cable);

    void setPath(const QPainterPath &path);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

private slots:
    void updatePen();

private:
    CircuitCable *mCable;
    QPen pen;
    QPainterPath mPath;
    QRectF mBoundingRect;
};

#endif // CABLEGRAPHITEM_H
