#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>

#include <unordered_map>

#include "../tilerotate.h"

class AbstractNodeGraphItem;

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

    void addNode(AbstractNodeGraphItem *node);

    TileLocation getNewFreeLocation();

signals:
    void modeChanged(Mode mode);

private:
    friend class AbstractNodeGraphItem;
    bool isLocationFree(TileLocation l) const;
    void updateItemLocation(TileLocation oldLocation,
                            TileLocation newLocation,
                            AbstractNodeGraphItem *item);

private:
    Mode mMode = Mode::Editing;

    std::unordered_map<TileLocation, AbstractNodeGraphItem *, TileLocationHash> mItemMap;
};

#endif // CIRCUITSCENE_H
