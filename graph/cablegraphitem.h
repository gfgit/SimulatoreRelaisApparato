#ifndef CABLEGRAPHITEM_H
#define CABLEGRAPHITEM_H

#include <QGraphicsObject>
#include <QPainterPath>
#include <QPen>
#include <QVector>

#include "../tilerotate.h"

class CircuitCable;
class CircuitScene;

class QJsonObject;

class CableGraphPath
{
public:
    CableGraphPath() = default;

    inline const QVector<TileLocation>& tiles() const
    {
        return mTiles;
    }

    inline int getTilesCount() const
    {
        return mTiles.size();
    }

    inline bool isEmpty() const
    {
        return mTiles.isEmpty();
    }

    inline TileLocation first() const { return mTiles.first(); }
    inline TileLocation last() const { return mTiles.last(); }
    inline TileLocation at(int i) const { return mTiles.at(i); }

    Connector::Direction getEnterDirection(int tileIdx) const;
    Connector::Direction getExitDirection(int tileIdx) const;

    bool addTile(const TileLocation& l);

    Connector::Direction startDirection() const;
    bool setStartDirection(Connector::Direction newStartDirection);

    Connector::Direction endDirection() const;
    bool setEndDirection(Connector::Direction newEndDirection);

    bool removeLastLine();

    QPainterPath generatePath() const;

    inline bool isZeroLength() const
    {
        return mIsZeroLength;
    }

    inline bool isComplete() const
    {
        return mPathIsComplete;
    }

    static CableGraphPath createZeroLength(const TileLocation& a,
                                           const TileLocation& b);

private:
    static Connector::Direction getDirection(const TileLocation& a,
                                             const TileLocation& b);

private:
    QVector<TileLocation> mTiles;
    Connector::Direction mStartDirection;
    Connector::Direction mEndDirection;

    bool mExitDirectionIsFree = true;
    Connector::Direction mWantedExitDirection = Connector::Direction::South;

    bool mPathIsComplete = false;
    bool mIsZeroLength = false;
};

class CableGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    CableGraphItem(CircuitCable *cable_);

    CircuitScene *circuitScene() const;

    QPainterPath path() const;

    const CableGraphPath& cablePath() const;
    void setCablePath(const CableGraphPath &newCablePath);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    CircuitCable *cable() const;

    bool cableZeroLength() const;

    TileLocation sideA() const;

    TileLocation sideB() const;

    Connector::Direction directionA() const;

    Connector::Direction directionB() const;

    bool loadFromJSON(const QJsonObject& obj);
    void saveToJSON(QJsonObject& obj) const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev);

private slots:
    void updatePen();
    void triggerUpdate();

private:
    CircuitCable *mCable;
    QPen pen;
    QPainterPath mPath;
    QRectF mBoundingRect;
    CableGraphPath mCablePath;
};

#endif // CABLEGRAPHITEM_H
