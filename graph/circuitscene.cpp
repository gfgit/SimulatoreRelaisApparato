#include "circuitscene.h"

#include "abstractnodegraphitem.h"

#include "powersourcegraphitem.h"
#include "../nodes/powersourcenode.h"

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene{parent}
{

}

CircuitScene::Mode CircuitScene::mode() const
{
    return mMode;
}

void CircuitScene::setMode(Mode newMode)
{
    if (mMode == newMode)
        return;
    mMode = newMode;
    emit modeChanged(mMode);

    const bool itemMovable = mMode == Mode::Editing;
    for(auto it = mItemMap.cbegin(); it != mItemMap.cend(); it++)
    {
        AbstractNodeGraphItem *node = it->second;
        node->setFlag(QGraphicsItem::ItemIsMovable, itemMovable);
    }

    const bool powerSourceEnabled = mMode == Mode::Simulation;
    for(PowerSourceGraphItem *powerSource : mPowerSources)
    {
        powerSource->node()->setEnabled(powerSourceEnabled);
    }
}

void CircuitScene::addNode(AbstractNodeGraphItem *item)
{
    addItem(item);

    if(!isLocationFree(item->location()))
    {
        // Assign a new location to node
        item->setLocation(getNewFreeLocation());
    }

    mItemMap.insert({item->location(), item});
    item->mLastValidLocation = item->location();

    item->setFlag(QGraphicsItem::ItemIsMovable, mMode == Mode::Editing);

    PowerSourceGraphItem *powerSource = qobject_cast<PowerSourceGraphItem *>(item);
    if(powerSource)
    {
        powerSource->node()->setEnabled(mMode == Mode::Simulation);
        mPowerSources.append(powerSource);
    }
}

TileLocation CircuitScene::getNewFreeLocation()
{
    TileLocation location{0, 0};
    while(!isLocationFree(location))
        location.x++;
    return location;
}

bool CircuitScene::isLocationFree(TileLocation l) const
{
    return mItemMap.find(l) == mItemMap.cend();
}

void CircuitScene::updateItemLocation(TileLocation oldLocation, TileLocation newLocation, AbstractNodeGraphItem *item)
{
    mItemMap.erase(oldLocation);
    mItemMap.insert({newLocation, item});
}
