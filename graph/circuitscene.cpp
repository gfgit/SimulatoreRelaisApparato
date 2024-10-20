#include "circuitscene.h"

#include "abstractnodegraphitem.h"

#include "powersourcegraphitem.h"
#include "../nodes/powersourcenode.h"
#include "../graph/cablegraphitem.h"
#include "../circuitcable.h"

#include <QGraphicsPathItem>
#include <QPen>

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

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

    Mode oldMode = mMode;
    mMode = newMode;
    emit modeChanged(mMode);

    const bool itemMovable = mMode == Mode::Editing;
    for(auto it = mItemMap.cbegin(); it != mItemMap.cend(); it++)
    {
        AbstractNodeGraphItem *node = it->second;
        node->setFlag(QGraphicsItem::ItemIsMovable, itemMovable);
    }

    if(oldMode == Mode::Editing)
    {
        if(isEditingCable())
            endEditCable(false);

        calculateConnections();
    }

    const bool powerSourceEnabled = mMode == Mode::Simulation;
    for(PowerSourceGraphItem *powerSource : mPowerSources)
    {
        powerSource->node()->setEnabled(powerSourceEnabled);
    }
}

void CircuitScene::addNode(AbstractNodeGraphItem *item)
{
    connect(item, &AbstractNodeGraphItem::editRequested,
            this, &CircuitScene::nodeEditRequested );

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

void CircuitScene::addCable(CableGraphItem *item)
{
    Q_ASSERT(item->cable());

    connect(item, &CableGraphItem::editRequested,
            this, &CircuitScene::cableEditRequested);

    // TODO: remove
    connect(item, &CableGraphItem::editRequested,
            [this, item]()
    {
        startEditCable(item);
    });

    addItem(item);
    mCables.insert({item->cable(), item});
}

void CircuitScene::removeCable(CircuitCable *cable)
{
    auto it = mCables.find(cable);
    if(it != mCables.end())
    {
        // Delete graph item
        CableGraphItem *item = it->second;
        mCables.erase(it);

        if(item == mEditingCable)
            endEditCable(false);

        delete item;
    }

    delete cable;
}

CableGraphItem *CircuitScene::graphForCable(CircuitCable *cable) const
{
    auto it = mCables.find(cable);
    if(it == mCables.end())
        return nullptr;
    return it->second;
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

void CircuitScene::calculateConnections()
{
    QVector<CircuitCable *> verifiedCables;

    auto cableCopy = mCables;
    for(const auto& it : cableCopy)
    {
        CableGraphItem *item = it.second;
        if(checkCable(item))
        {
            verifiedCables.append(item->cable());
        }
        else
        {
            if(item->cableZeroLength())
            {
                // Unconnected zero length cable, delete it
                removeCable(item->cable());
            }
        }
    }

    for(const auto& it : mItemMap)
    {
        AbstractNodeGraphItem *item = it.second;
        TileLocation location = item->location();

        AbstractCircuitNode *node1 = item->getAbstractNode();
        if(!node1)
            continue;

        std::vector<Connector> connectors;
        item->getConnectors(connectors);

        for(const Connector& c1 : connectors)
        {
            TileLocation otherLocation = location + c1.direction;
            AbstractNodeGraphItem *other = getItemAt(otherLocation);
            AbstractCircuitNode *node2 = other ? other->getAbstractNode() : nullptr;

            if(!node2)
            {
                // Detach our cable if any
                auto cableA = node1->getContacts().at(c1.nodeContact).cable;
                if(cableA && !verifiedCables.contains(cableA))
                {
                    node1->detachCable(c1.nodeContact);

                    auto item = graphForCable(cableA);
                    if(!item || item->cableZeroLength())
                    {
                        removeCable(cableA);
                        cableA = nullptr;
                    }
                }
            }

            if(!other)
                continue;

            std::vector<Connector> otherConnectors;
            other->getConnectors(otherConnectors);

            for(const Connector& c2 : otherConnectors)
            {
                if(c2.direction != ~c1.direction)
                    continue;

                // We have a match

                if(!node1 || !node2)
                    break;

                connectItems(node1, node2, c1, c2, verifiedCables);
                break;
            }
        }
    }

    // Delete unconnected zero length cables
    // Copy to avoid invalidating iterators while looping
    cableCopy = mCables;
    for(auto it = cableCopy.begin(); it != cableCopy.end(); it++)
    {
        CableGraphItem *item = it->second;
        if(item->cableZeroLength() && !verifiedCables.contains(item->cable()))
        {
            removeCable(item->cable());
        }
    }
}

void CircuitScene::connectItems(AbstractCircuitNode *node1, AbstractCircuitNode *node2, const Connector &c1, const Connector &c2, QVector<CircuitCable *> &verifiedCables)
{
    const auto& contacts1 = node1->getContacts();
    const auto& contacts2 = node2->getContacts();

    CircuitCable *cableA = contacts1.at(c1.nodeContact).cable;

    if(!cableA || !verifiedCables.contains(cableA))
    {
        CircuitCable *cableB = contacts2.at(c2.nodeContact).cable;
        if(cableB != cableA || !cableB)
        {
            if(cableA)
            {
                node1->detachCable(c1.nodeContact);

                auto item = graphForCable(cableA);
                if(!item || item->cableZeroLength())
                {
                    removeCable(cableA);
                    cableA = nullptr;
                }
            }

            if(cableB)
            {
                node2->detachCable(c2.nodeContact);

                auto item = graphForCable(cableB);
                if(!item || item->cableZeroLength())
                {
                    removeCable(cableB);
                    cableB = nullptr;
                }
            }

            CircuitCable *newCable = new CircuitCable(this);

            // First we set graph path
            CableGraphItem *item = new CableGraphItem(cableA);
            item->setPos(0, 0);
            QPainterPath path;
            path.moveTo(getConnectorPoint(c1.location, c1.direction));
            path.lineTo(getConnectorPoint(c2.location, c2.direction));
            item->setPath(path);

            // Then we create cable connection
            AbstractCircuitNode::CableItem cableItem;
            cableItem.cable.cable = newCable;
            cableItem.cable.side = CircuitCable::Side::A;
            cableItem.nodeContact = c1.nodeContact;
            cableItem.cable.pole = CircuitCable::Pole::First;
            node1->attachCable(cableItem);

            cableItem.cable.pole = CircuitCable::Pole::Second;
            node1->attachCable(cableItem);

            cableItem.cable.side = CircuitCable::Side::B;
            cableItem.nodeContact = c2.nodeContact;
            cableItem.cable.pole = CircuitCable::Pole::First;
            node2->attachCable(cableItem);

            cableItem.cable.pole = CircuitCable::Pole::Second;
            node2->attachCable(cableItem);

            cableA = newCable;
            verifiedCables.append(cableA);

            addCable(item);
        }
    }
}

bool CircuitScene::checkCable(CableGraphItem *item)
{
    if(item->sideA() == TileLocation::invalid || item->sideB() == TileLocation::invalid)
        return false;

    TileLocation nodeLocA = item->sideA() + item->directionA();
    TileLocation nodeLocB = item->sideB();
    if(!item->cableZeroLength())
        nodeLocB = nodeLocB + item->directionB();

    AbstractNodeGraphItem *itemA = getItemAt(nodeLocA);
    AbstractNodeGraphItem *itemB = getItemAt(nodeLocB);

    AbstractCircuitNode *nodeA = itemA ? itemA->getAbstractNode() : nullptr;
    AbstractCircuitNode *nodeB = itemB ? itemB->getAbstractNode() : nullptr;

    CircuitCable *cable = item->cable();

    std::vector<Connector> connectorsA;
    std::vector<Connector> connectorsB;

    std::vector<Connector>::const_iterator connA = connectorsA.cend();
    std::vector<Connector>::const_iterator connB = connectorsB.cend();

    if(itemA)
    {
        itemA->getConnectors(connectorsA);

        connA = std::find_if(connectorsA.cbegin(), connectorsA.cend(),
                             [item](const Connector& c)
        {
            return c.direction == ~item->directionA();
        });
    }

    if(itemB)
    {
        itemB->getConnectors(connectorsB);

        connB = std::find_if(connectorsB.cbegin(), connectorsB.cend(),
                             [item](const Connector& c)
        {
            return c.direction == ~item->directionB();
        });
    }

    if(!nodeA || connA == connectorsA.cend() || !nodeB || connB == connectorsB.cend())
    {
        // Detach side A
        CircuitCable::CableEnd end = cable->getNode(CircuitCable::Side::A);
        if(end.node)
        {
            end.node->detachCable(end.nodeContact);
        }

        // Detach side B
        end = cable->getNode(CircuitCable::Side::B);
        if(end.node)
        {
            end.node->detachCable(end.nodeContact);
        }

        return false;
    }

    const auto& contactA = nodeA->getContacts().at(connA->nodeContact);

    if(contactA.cable == item->cable() && contactA.cableSide == CircuitCable::Side::B)
    {
        // We have a swapped cable, detach and let it rewire later
        CircuitCable::CableEnd end = cable->getNode(CircuitCable::Side::A);
        if(end.node)
        {
            end.node->detachCable(end.nodeContact);
        }

        end = cable->getNode(CircuitCable::Side::B);
        if(end.node)
        {
            end.node->detachCable(end.nodeContact);
        }
    }

    bool sideConnectedA = false;
    if(contactA.cable == item->cable())
    {
        sideConnectedA = true;
    }
    else if(!contactA.cable && !item->cable()->getNode(CircuitCable::Side::A).node)
    {
        // Make the connection
        AbstractCircuitNode::CableItem cableItem;
        cableItem.cable.cable = item->cable();
        cableItem.cable.side = CircuitCable::Side::A;
        cableItem.nodeContact = connA->nodeContact;
        cableItem.cable.pole = CircuitCable::Pole::First;
        nodeA->attachCable(cableItem);

        cableItem.cable.pole = CircuitCable::Pole::Second;
        nodeA->attachCable(cableItem);

        sideConnectedA = true;
    }

    const auto& contactB = nodeB->getContacts().at(connB->nodeContact);

    bool sideConnectedB = false;
    if(contactB.cable == item->cable())
    {
        sideConnectedB = true;
    }
    else if(!contactB.cable && !item->cable()->getNode(CircuitCable::Side::B).node)
    {
        // Make the connection
        AbstractCircuitNode::CableItem cableItem;
        cableItem.cable.cable = item->cable();
        cableItem.cable.side = CircuitCable::Side::B;
        cableItem.nodeContact = connB->nodeContact;
        cableItem.cable.pole = CircuitCable::Pole::First;
        nodeB->attachCable(cableItem);

        cableItem.cable.pole = CircuitCable::Pole::Second;
        nodeB->attachCable(cableItem);

        sideConnectedB = true;
    }

    return sideConnectedA && sideConnectedB;
}

QPointF CircuitScene::getConnectorPoint(TileLocation l, Connector::Direction direction)
{
    const QPointF pos = l.toPoint();

    switch (direction)
    {
    case Connector::Direction::North:
        return {pos.x() + TileLocation::HalfSize, pos.y()};
    case Connector::Direction::South:
        return {pos.x() + TileLocation::HalfSize, pos.y() + TileLocation::Size};
    case Connector::Direction::East:
        return {pos.x() + TileLocation::Size, pos.y() + TileLocation::HalfSize};
    case Connector::Direction::West:
        return {pos.x(), pos.y() + TileLocation::HalfSize};
    default:
        break;
    }

    return QPointF();
}

void CircuitScene::startEditCable(CableGraphItem *item)
{
    if(isEditingCable())
        endEditCable(false);

    mEditingCable = item;

    QPen pen;
    pen.setWidthF(6.0);
    pen.setColor(Qt::darkGreen);
    mEditOverlay = addPath(mEditingCable->path(), pen);
    mEditOverlay->setZValue(1.0);

    pen.setColor(Qt::red);
    mEditNewPath = addPath(QPainterPath(), pen);
    mEditNewPath->setZValue(2.0);

    mEditPathEmpty = true;
}

void CircuitScene::endEditCable(bool apply)
{
    if(!isEditingCable())
        return;

    if(apply && !mEditNewPath->path().isEmpty())
        mEditingCable->setPath(mEditNewPath->path());

    mEditingCable = nullptr;
    delete mEditOverlay;
    delete mEditNewPath;
}

void CircuitScene::editCableAddPoint(const QPointF &p)
{
    int16_t hx = static_cast<int16_t>(std::round(p.x() / TileLocation::HalfSize));
    int16_t hy = static_cast<int16_t>(std::round(p.y() / TileLocation::HalfSize));

    TileLocation location{hx / 2, hy / 2};

    QPointF realPoint(hx * TileLocation::HalfSize,
                      hy * TileLocation::HalfSize);

    const bool isEdge = (hy % 2) != (hx % 2);

    Connector::Direction direction = Connector::Direction::North;
    if(hy % 2 == 1)
    {
        if(isEdge)
            direction = Connector::Direction::West;
        location.y = (hy - 1) / 2;
    }
    else
    {
        location.y = hy / 2;
        if(p.y() < realPoint.y())
        {
            location.y--;
            if(isEdge)
                direction = Connector::Direction::South;
        }
    }

    if(hx % 2 == 1)
    {
        location.x = (hx - 1) / 2;
    }
    else
    {
        location.x = hx / 2;
        if(p.x() < realPoint.x())
        {
            location.x--;
            if(isEdge)
                direction = Connector::Direction::East;
        }
    }

    QPointF tileCenter = location.toPoint();
    tileCenter.rx() += TileLocation::HalfSize;
    tileCenter.ry() += TileLocation::HalfSize;

    QPainterPath path = mEditNewPath->path();
    if(mEditPathEmpty && isEdge)
    {
        mEditPathEmpty = false;
        path.moveTo(realPoint);
        path.lineTo(tileCenter);
    }
    else if(!mEditPathEmpty)
    {
        auto el = path.elementAt(path.elementCount() - 1);
        bool sameX = qFuzzyCompare(tileCenter.x(), el.x);
        bool sameY = qFuzzyCompare(tileCenter.y(), el.y);

        if(sameX || sameY)
        {
            if(!(sameX && sameY))
            {
                path.lineTo(tileCenter);
            }

            path.lineTo(realPoint);
        }
    }

    mEditNewPath->setPath(path);
}

void CircuitScene::editCableUndoLast()
{
    if(!isEditingCable())
        return;

    QPainterPath path = mEditNewPath->path();
    QPainterPath newPath;
    if(path.elementCount() >= 3)
    {
        // Copy all elements except last to newPath
        // If less than 3 elements, leave empty

        const int newCount = path.elementCount() - 1;
        for(int i = 0; i < newCount; i++)
        {
            const QPainterPath::Element& e = path.elementAt(i);
            if(e.isMoveTo())
                newPath.moveTo(e.x, e.y);
            else if(e.isLineTo())
                newPath.lineTo(e.x, e.y);
        }
    }
    else
    {
        mEditPathEmpty = true;
    }

    mEditNewPath->setPath(newPath);
}

void CircuitScene::keyReleaseEvent(QKeyEvent *e)
{
    if(isEditingCable())
    {
        bool consumed = true;

        switch (e->key())
        {
        case Qt::Key_Escape:
            endEditCable(false);
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            endEditCable(true);
            break;
        default:
            consumed = false;
            break;
        }

        if(!consumed && e->matches(QKeySequence::Undo))
        {
            consumed = true;

            editCableUndoLast();
        }

        if(consumed)
        {
            e->accept();
            return;
        }
    }

    QGraphicsScene::keyReleaseEvent(e);
}

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if(isEditingCable() && e->button() == Qt::LeftButton)
    {
        editCableAddPoint(e->scenePos());
        e->accept();
        return;
    }

    QGraphicsScene::mousePressEvent(e);
}
