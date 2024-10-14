#ifndef ONOFFGRAPHITEM_H
#define ONOFFGRAPHITEM_H


#include <QGraphicsObject>

class OnOffSwitchNode;

class OnOffGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    OnOffGraphItem(OnOffSwitchNode *node);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

private slots:
    void triggerUpdate();

private:
    OnOffSwitchNode *mNode;
};

#endif // ONOFFGRAPHITEM_H
