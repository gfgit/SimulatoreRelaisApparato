#ifndef ABSTRACTNODEGRAPHITEM_H
#define ABSTRACTNODEGRAPHITEM_H

#include <QGraphicsObject>

#include "../tilerotate.h"

class AbstractCircuitNode;
class CircuitScene;

class QJsonObject;

class AbstractNodeGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    AbstractNodeGraphItem(AbstractCircuitNode *node_);

    QRectF boundingRect() const override;

    virtual void getConnectors(std::vector<Connector>& /*connectors*/) const {}

    inline AbstractCircuitNode *getAbstractNode() const
    {
        return mAbstractNode;
    }

    inline TileLocation location() const
    {
        return TileLocation::fromPoint(pos());
    }

    inline void setLocation(const TileLocation& l)
    {
        setPos(l.toPoint());
    }

    TileRotate rotate() const;
    void setRotate(TileRotate newRotate);

    CircuitScene *circuitScene() const;

    virtual bool loadFromJSON(const QJsonObject& obj);
    virtual void saveToJSON(QJsonObject& obj) const;

protected slots:
    void triggerUpdate();
    virtual void updateName();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void drawMorsetti(QPainter *painter, int nodeContact, TileRotate r);
    void drawName(QPainter *painter, const QString &name, TileRotate r);

    void invalidateConnections(bool tryReconnectImmediately = false);

private:
    AbstractCircuitNode *mAbstractNode;
    TileRotate mRotate = TileRotate::Deg0;
};

#endif // ABSTRACTNODEGRAPHITEM_H
