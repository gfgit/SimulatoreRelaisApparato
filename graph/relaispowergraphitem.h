#ifndef RELAISPOWERGRAPHITEM_H
#define RELAISPOWERGRAPHITEM_H


#include <QGraphicsObject>

class RelaisPowerNode;
class AbstractRelais;

class RelaisPowerGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    RelaisPowerGraphItem(RelaisPowerNode *node);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

private slots:
    void triggerUpdate();
    void updateRelay();
    void updateName();

private:
    RelaisPowerNode *mNode;
    AbstractRelais *mRelay = nullptr;
};

#endif // RELAISPOWERGRAPHITEM_H
