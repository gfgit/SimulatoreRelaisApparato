#ifndef ABSTRACTNODEGRAPHITEM_H
#define ABSTRACTNODEGRAPHITEM_H

#include <QGraphicsObject>

#include "../tilerotate.h"

class AbstractCircuitNode;

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

protected slots:
    void triggerUpdate();
    virtual void updateName();

protected:
    void drawMorsetti(QPainter *painter, bool on, const QString &name1, const QString &name2, TileRotate r);
    void drawName(QPainter *painter, const QString &name, TileRotate r);

private:
    AbstractCircuitNode *mAbstractNode;
    TileRotate mRotate = TileRotate::Deg0;
};

#endif // ABSTRACTNODEGRAPHITEM_H
