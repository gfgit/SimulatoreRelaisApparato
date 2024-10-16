#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>

#include <unordered_map>

#include "../tilerotate.h"

class AbstractCircuitNode;
class AbstractNodeGraphItem;
class PowerSourceGraphItem;
class Connector;
class CircuitCable;
class CableGraphItem;

class CircuitScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum class Mode
    {
        Static = 0,
        Simulation,
        Editing
    };

    explicit CircuitScene(QObject *parent = nullptr);

    Mode mode() const;
    void setMode(Mode newMode);

    void addNode(AbstractNodeGraphItem *item);
    void addCable(CableGraphItem *item);
    void removeCable(CircuitCable *cable);
    CableGraphItem *graphForCable(CircuitCable *cable) const;

    TileLocation getNewFreeLocation();

    static QPointF getConnectorPoint(TileLocation l, Connector::Direction direction);

signals:
    void modeChanged(Mode mode);
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

private:
    friend class AbstractNodeGraphItem;
    bool isLocationFree(TileLocation l) const;
    void updateItemLocation(TileLocation oldLocation,
                            TileLocation newLocation,
                            AbstractNodeGraphItem *item);

    void calculateConnections();
    void connectItems(AbstractCircuitNode *node1,
                      AbstractCircuitNode *node2,
                      const Connector& c1, const Connector& c2,
                      QVector<CircuitCable *>& verifiedCables);

    bool checkCable(CableGraphItem *item);

    inline AbstractNodeGraphItem *getItemAt(TileLocation l) const
    {
        auto it = mItemMap.find(l);
        if(it != mItemMap.cend())
            return it->second;
        return nullptr;
    }

private:
    Mode mMode = Mode::Editing;

    std::unordered_map<TileLocation, AbstractNodeGraphItem *, TileLocationHash> mItemMap;

    QVector<PowerSourceGraphItem *> mPowerSources;

    std::unordered_map<CircuitCable *, CableGraphItem *> mCables;
};

#endif // CIRCUITSCENE_H
