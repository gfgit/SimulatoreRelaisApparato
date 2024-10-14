#ifndef POWERSOURCEGRAPHITEM_H
#define POWERSOURCEGRAPHITEM_H


#include <QGraphicsObject>

class PowerSourceNode;

class PowerSourceGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    PowerSourceGraphItem(PowerSourceNode *node);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

private slots:
    void triggerUpdate();
    void updateName();

private:
    PowerSourceNode *mNode;
};

#endif // POWERSOURCEGRAPHITEM_H
