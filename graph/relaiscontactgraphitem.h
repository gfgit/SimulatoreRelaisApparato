#ifndef RELAISCONTACTGRAPHITEM_H
#define RELAISCONTACTGRAPHITEM_H


#include <QGraphicsObject>

class RelaisContactNode;

class RelaisContactGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    RelaisContactGraphItem(RelaisContactNode *node);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

private slots:
    void triggerUpdate();
    void updateName();

private:
    RelaisContactNode *mNode;
};

#endif // RELAISCONTACTGRAPHITEM_H
