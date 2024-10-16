#include "circuitscene.h"

#include "abstractnodegraphitem.h"

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

    bool itemMovable = mMode == Mode::Editing;
    for(auto it = mItemMap.cbegin(); it != mItemMap.cend(); it++)
    {
        AbstractNodeGraphItem *node = it->second;
        node->setFlag(QGraphicsItem::ItemIsMovable, itemMovable);
    }
}

void CircuitScene::addNode(AbstractNodeGraphItem *node)
{
    addItem(node);

    if(!isLocationFree(node->location()))
    {
        // Assign a new location to node
        node->setLocation(getNewFreeLocation());
    }

    mItemMap.insert({node->location(), node});
    node->mLastValidLocation = node->location();

    node->setFlag(QGraphicsItem::ItemIsMovable, mMode == Mode::Editing);
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
