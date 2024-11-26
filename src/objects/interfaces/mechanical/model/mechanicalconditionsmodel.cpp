/**
 * src/objects/interfaces/mechanical/model/mechanicalconditionsmodel.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mechanicalconditionsmodel.h"

#include "../../mechanicalinterface.h"
#include "../../../abstractsimulationobject.h"

MechanicalConditionsModel::MechanicalConditionsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // Fake item for invisible root
    rootItem.type = Item::Type::ExactPos;
}

QVariant MechanicalConditionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case ObjectCol:
            return tr("Object");
        case TypeCol:
            return tr("Type");
        case FromCol:
            return tr("From");
        case ToCol:
            return tr("To");
        default:
            break;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex MechanicalConditionsModel::index(int row, int column, const QModelIndex &p) const
{
    if (!hasIndex(row, column, p))
        return {};

    const Item *parentItem = p.isValid()
            ? static_cast<const Item*>(p.internalPointer())
            : &rootItem;

    if (auto *childItem = parentItem->subConditions.at(row))
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex MechanicalConditionsModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    auto *childItem = static_cast<Item *>(child.internalPointer());
    Item *parentItem = childItem->parent;

    return parentItem != &rootItem
            ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}


int MechanicalConditionsModel::rowCount(const QModelIndex &p) const
{
    if (p.column() >= NCols)
        return 0;

    const Item *parentItem = p.isValid()
            ? static_cast<const Item*>(p.internalPointer())
            : &rootItem;

    return parentItem->subConditions.size();
}

int MechanicalConditionsModel::columnCount(const QModelIndex &p) const
{
    Q_UNUSED(p)
    return NCols;
}

QVariant MechanicalConditionsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || !idx.internalPointer())
        return QVariant();

    const Item *item = static_cast<const Item *>(idx.internalPointer());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case ObjectCol:
        {
            if(item->otherIface)
                return item->otherIface->object()->name();
            break;
        }
        case TypeCol:
        {
            return MechanicalCondition::getTypeDesc().name(int(item->type));
        }
        case FromCol:
        {
            if(item->otherIface)
                return item->otherIface->positionDesc().name(item->requiredPositions.first);
            break;
        }
        case ToCol:
            if(item->type != Item::Type::RangePos)
                return QString();
            if(item->otherIface)
                return item->otherIface->positionDesc().name(item->requiredPositions.second);
            break;
        default:
            break;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TypeCol:
        {
            return int(item->type);
        }
        case FromCol:
        {
            return item->requiredPositions.first;
        }
        case ToCol:
        {
            return item->requiredPositions.second;
        }
        default:
            break;
        }
    }
    default:
        break;
    }

    return QVariant();
}

bool MechanicalConditionsModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid() || !idx.internalPointer())
        return false;

    Item *item = static_cast<Item *>(idx.internalPointer());

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TypeCol:
        {
            const Item::Type type = Item::Type(value.toInt());
            if(item->type == type)
                return false;

            if(!allowedTypesFor(idx).contains(int(type)))
                return false;

            // Set new type
            item->type = Item::Type(type);

            // Check position values
            if(item->type == Item::Type::RangePos)
            {
                // For Range items ensure second is greater than first

                auto allowedRange = allowedPositionsFor(idx.siblingAtColumn(FromCol));
                if(!allowedRange.isEmpty())
                {
                    item->requiredPositions.first = qBound(allowedRange.first(),
                                                           item->requiredPositions.first,
                                                           allowedRange.last());
                }

                allowedRange = allowedPositionsFor(idx.siblingAtColumn(ToCol));
                if(!allowedRange.isEmpty())
                {
                    item->requiredPositions.second = qBound(allowedRange.first(),
                                                            item->requiredPositions.second,
                                                            allowedRange.last());
                }
            }
            else
            {
                // Non-Range items have both position equal
                item->requiredPositions.second = item->requiredPositions.first;
            }

            if(item->type == Item::Type::And &&
                    item->parent->type == Item::Type::And)
            {
                const auto subCondCopy = item->subConditions;
                for(Item *sub : subCondCopy)
                {
                    if(sub->type == Item::Type::And)
                    {
                        // There is a sub nested AND, remove it
                        moveChildrenUpAndRemoveParent(sub, index(sub->row(), 0, idx));
                    }
                }

                if(item->parent->type == Item::Type::And)
                {
                    // We are a nested AND, remove
                    moveChildrenUpAndRemoveParent(item, idx);

                    // Since we removed ourselves we do not emit dataChanged() signal
                    return true;
                }
            }
            break;
        }
        case FromCol:
        case ToCol:
        {
            const int newPosition = value.toInt();
            int &position = idx.column() == FromCol ?
                        item->requiredPositions.first :
                        item->requiredPositions.second;

            if(newPosition == position)
                return false;

            if(!allowedPositionsFor(idx).contains(newPosition))
                return false;

            if(idx.column() == FromCol)
            {
                if(item->type == Item::Type::RangePos)
                {
                    // For Range items ensure second is greater than first
                    const auto allowedPositions = allowedPositionsFor(idx.siblingAtColumn(ToCol));
                    if(allowedPositions.isEmpty())
                    {
                        // We cannot set range end so we revert the change
                        return false;
                    }

                    item->requiredPositions.second = qMax(item->requiredPositions.second,
                                                          allowedPositions.first());
                }
                else
                {
                    // Non-Range items have both position equal
                    item->requiredPositions.second = item->requiredPositions.first;
                }
            }

            // Set new position
            position = newPosition;
        }
        default:
            break;
        }
    }
    default:
        break;
    }

    emit changed();

    emit dataChanged(idx.siblingAtColumn(TypeCol),
                     idx.siblingAtColumn(ToCol));
    return true;
}

Qt::ItemFlags MechanicalConditionsModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.column() >= NCols || !idx.internalPointer())
        return Qt::NoItemFlags;

    const Item *item = static_cast<const Item *>(idx.internalPointer());

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if(item->type == Item::Type::And || item->type == Item::Type::Or)
    {
        // ANDs and ORs
        // Only TypeCol can be edited
        if(idx.column() == TypeCol)
            f.setFlag(Qt::ItemIsEditable);

        return f;
    }

    // Normal items
    if(!item->otherIface)
    {
        // Interface is not set, must set it first
        if(idx.column() == ObjectCol)
            f.setFlag(Qt::ItemIsEditable);

        return f;
    }

    // When interface is set, also other colums are editable
    // Except for the ToCol which is editable only for Range conditions
    if(idx.column() != ToCol || item->type == Item::Type::RangePos)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

MechanicalCondition MechanicalConditionsModel::getConditionTree() const
{
    if(rootItem.subConditions.isEmpty())
        return MechanicalCondition();

    const Item *item = rootItem.subConditions.constFirst();
    return recursiveBuildConditions(item);
}

void MechanicalConditionsModel::setConditionTree(const MechanicalCondition &newConditions)
{
    beginResetModel();

    qDeleteAll(rootItem.subConditions);
    rootItem.subConditions.clear();

    MechanicalCondition c = newConditions;
    c.removeInvalidConditions();
    c.simplifyTree();

    if(c != MechanicalCondition())
        recursiveParseTree(&rootItem, c);

    endResetModel();
}

QModelIndex MechanicalConditionsModel::instertConditionAt(const QModelIndex& idx, Item::Type type)
{
    Item *item = nullptr;

    // Children are always related to first column
    // beginInsertRows() does not work if another column is passed
    QModelIndex actualIdx = idx.siblingAtColumn(0);

    if(idx == QModelIndex())
    {
        if(rootItem.subConditions.isEmpty())
        {
            // Add first and only root child
            beginInsertRows(QModelIndex(), 0, 0);
            Item *child = new Item;
            child->type = type;
            child->parent = &rootItem;
            rootItem.subConditions.append(child);
            endInsertRows();

            emit changed();

            // Do not add other root children
            return index(0, 0, QModelIndex());
        }

        // Root can have only one child so always
        // manipulate it's first child instead
        item = rootItem.subConditions.first();
        actualIdx = index(0, 0, QModelIndex());
    }
    else
    {
        if(!idx.internalPointer())
            return actualIdx;

        item = static_cast<Item *>(idx.internalPointer());

        if(type == Item::Type::And && item->type != type && item->type != Item::Type::Or && type == item->parent->type)
        {
            // Do not add nested ANDs
            item = item->parent;
            actualIdx = parent(actualIdx);
        }
    }

    if(item->type == Item::Type::And || item->type == Item::Type::Or)
    {
        // Add a child to current item
        const int row = item->subConditions.size();

        beginInsertRows(actualIdx, row, row);

        Item *child = new Item;
        if(type == Item::Type::And && item->type == type)
        {
            // Do not add nested ANDs
            type = Item::Type::ExactPos;
        }

        if(type == Item::Type::And || type == Item::Type::Or)
        {
            // Add 2 empty children by default to new ORs and ANDs
            Item *subChild = new Item;
            subChild->type = Item::Type::ExactPos;
            subChild->parent = child;
            child->subConditions.append(subChild);

            subChild = new Item;
            subChild->type = Item::Type::ExactPos;
            subChild->parent = child;
            child->subConditions.append(subChild);
        }

        child->type = type;
        child->parent = item;
        item->subConditions.insert(row, child);

        endInsertRows();

        emit changed();

        return actualIdx;
    }

    // Normal item
    if(type != Item::Type::And && type != Item::Type::Or)
        return actualIdx; // Cannot add a normal item as child of normal item

    // Morph into AND or OR as requested, move original to child
    Item *child = new Item;
    *child = *item; // Copy item to new child

    const QModelIndex p = parent(actualIdx);
    const int origRow = item->row();

    // Add AND/OR item to parent just before current item,
    // it will then replace current item
    beginInsertRows(p, origRow, origRow);
    Item *replacement = new Item;
    replacement->type = type;
    replacement->parent = item->parent;
    item->parent->subConditions.insert(origRow, replacement);
    endInsertRows();

    const int itemRow = item->row();
    const QPersistentModelIndex replacementIdx = index(replacement->row(), 0, p);

    // Move current item (which in the meantime was shifted by one row)
    // from parent to child of it's replacement
    beginMoveRows(p, itemRow, itemRow,
                  replacementIdx, 0);

    // Reparent
    item->parent->subConditions.removeAt(itemRow);
    item->parent = replacement;
    replacement->subConditions.append(item);

    endMoveRows();

    // Add an extra empty child by default to new ORs and ANDs
    const int subChildRow = replacement->subConditions.size();
    beginInsertRows(replacementIdx, subChildRow, subChildRow);
    Item *subChild = new Item;
    subChild->type = Item::Type::ExactPos;
    subChild->parent = replacement;
    replacement->subConditions.append(subChild);
    endInsertRows();

    emit changed();

    return replacementIdx;
}

void MechanicalConditionsModel::removeConditionAt(const QModelIndex& idx)
{
    if(!idx.isValid() || !idx.internalPointer())
        return;

    Item *item = static_cast<Item *>(idx.internalPointer());
    const int origRow = item->row();

    Item *parentItem = item->parent;
    QModelIndex parentIdx = parent(idx);

    beginRemoveRows(parentIdx, origRow, origRow);

    // Unregister from parent
    parentItem->subConditions.removeAt(origRow);
    delete item;

    endRemoveRows();

    // Now simplify parent recursively
    while(parentItem != &rootItem)
    {
        if(parentItem->subConditions.size() > 1)
            break;

        Item *grandParentItem = parentItem->parent;
        QModelIndex grandParentIdx = moveChildrenUpAndRemoveParent(parentItem, parentIdx);

        // Check level above
        parentItem = grandParentItem;
        parentIdx = grandParentIdx;
    }

    emit changed();
}

EnumDesc MechanicalConditionsModel::getPositionDescFor(const QModelIndex &idx) const
{
    if(!idx.isValid() || !idx.internalPointer())
        return {};

    const Item *item = static_cast<const Item *>(idx.internalPointer());
    if(!item->otherIface)
        return {};

    if(idx.column() == FromCol || idx.column() == ToCol)
    {
        return item->otherIface->positionDesc();
    }

    return {};
}

QVector<int> MechanicalConditionsModel::allowedPositionsFor(const QModelIndex &idx) const
{
    if(!idx.isValid() || !idx.internalPointer())
        return {};

    const Item *item = static_cast<const Item *>(idx.internalPointer());
    if(!item->otherIface)
        return {};

    int minVal = 0;
    int maxVal = item->otherIface->absoluteMax();

    if(idx.column() == FromCol)
    {
        minVal = item->otherIface->absoluteMin();

        // For Range items, we cannot set max as "From"
        // because "To" must be greater than "From"
        if(item->type == Item::Type::RangePos)
            maxVal--;
    }
    else if(idx.column() == ToCol)
    {
        // "To" position must be greater than "From"
        minVal = item->requiredPositions.first + 1;
    }
    else
    {
        return {};
    }

    QVector<int> result;
    for(int pos : item->otherIface->lockablePositions())
    {
        if(pos < minVal || pos > maxVal)
            continue; // Skip

        result.append(pos);
    }
    return result;
}

QVector<int> MechanicalConditionsModel::allowedTypesFor(const QModelIndex &idx) const
{
    if(!idx.isValid() || !idx.internalPointer() || idx.column() != TypeCol)
        return {};

    const Item *item = static_cast<const Item *>(idx.internalPointer());
    if(item->type == Item::Type::And || item->type == Item::Type::Or)
    {
        // Grouping items will remain grouping items
        return {int(Item::Type::And), int(Item::Type::Or)};
    }

    // Normal items
    if(!item->otherIface)
        return {};

    QVector<int> result;
    for(MechanicalCondition::Type type : item->otherIface->allowedConditionTypes())
    {
        result.append(int(type));
    }
    return result;
}

AbstractSimulationObject *MechanicalConditionsModel::getObjectAt(const QModelIndex &idx) const
{
    if(!idx.isValid() || !idx.internalPointer())
        return nullptr;

    const Item *item = static_cast<const Item *>(idx.internalPointer());
    if(!item->otherIface)
        return nullptr;

    return item->otherIface->object();
}

bool MechanicalConditionsModel::setObjectAt(const QModelIndex &idx, AbstractSimulationObject *obj)
{
    if(!idx.isValid() || !idx.internalPointer())
        return false;

    Item *item = static_cast<Item *>(idx.internalPointer());
    if(item->type == Item::Type::And || item->type == Item::Type::Or)
        return false; // Only normal items can have object

    if(!obj)
    {
        // Reset item
        item->otherIface = nullptr;
        item->type = Item::Type::ExactPos;
        item->requiredPositions = {0, 0};
    }
    else
    {
        MechanicalInterface *iface = obj->getInterface<MechanicalInterface>();
        if(!iface)
            return false;

        item->otherIface = iface;
        item->type = Item::Type(iface->allowedConditionTypes().first());
        item->requiredPositions.first = iface->lockablePositions().first();
        if(item->type == Item::Type::RangePos)
            item->requiredPositions.second = iface->lockablePositions().at(1);
    }

    emit dataChanged(idx, idx);
    return true;
}

void MechanicalConditionsModel::recursiveParseTree(Item *p, const MechanicalCondition &c)
{
    Item *item = new Item;
    item->parent = p;
    p->subConditions.append(item);

    item->type = Item::Type(c.type);

    if(item->type == Item::Type::And || item->type == Item::Type::Or)
    {
        for(const MechanicalCondition& sub : c.subConditions)
            recursiveParseTree(item, sub);
    }
    else
    {
        item->otherIface = c.otherIface;
        item->requiredPositions = c.requiredPositions;
    }
}

MechanicalCondition MechanicalConditionsModel::recursiveBuildConditions(const Item *item) const
{
    MechanicalCondition cond;
    cond.type = MechanicalCondition::Type(item->type);

    if(item->type == Item::Type::And || item->type == Item::Type::Or)
    {
        for(const Item *sub : item->subConditions)
        {
            MechanicalCondition child = recursiveBuildConditions(sub);
            cond.subConditions.append(child);
        }
    }
    else
    {
        if(!item->otherIface)
            return MechanicalCondition();

        cond.otherIface = item->otherIface;
        cond.requiredPositions = item->requiredPositions;
    }

    return cond;
}

QModelIndex MechanicalConditionsModel::moveChildrenUpAndRemoveParent(Item *parentItem, const QModelIndex &parentIdx)
{
    Item *grandParentItem = parentItem->parent;
    QModelIndex grandParentIdx = parent(parentIdx);

    // Replace parent with its children in 2 steps
    // First we move children to same level of parent just before it
    // Then we remove parent
    int parentRow = parentItem->row();
    const int childrenCount = parentItem->subConditions.size();

    int grandParentInsertRow = parentRow;

    // Move child up by one level, just before it's parent
    beginMoveRows(parentIdx, 0, childrenCount - 1,
                  grandParentIdx, parentRow);

    // Reparent
    for(Item *child : std::as_const(parentItem->subConditions))
    {
        child->parent = grandParentItem;
        grandParentItem->subConditions.insert(grandParentInsertRow, child);
        grandParentInsertRow++;
    }
    parentItem->subConditions.clear();

    endMoveRows();

    // Now remove old parent (it's now empty and shifted by childrenCount rows)
    parentRow = parentItem->row(); // Recalculate

    beginRemoveRows(grandParentIdx, parentRow, parentRow);

    grandParentItem->subConditions.removeAt(parentRow);
    delete parentItem;
    parentItem = nullptr;

    endRemoveRows();

    return grandParentIdx;
}

int MechanicalConditionsModel::Item::row() const
{
    if (parent == nullptr)
        return 0;

    const auto it = std::find_if(parent->subConditions.cbegin(),
                                 parent->subConditions.cend(),
                                 [this](const Item *item)
    {
        return item == this;
    });

    if (it != parent->subConditions.cend())
        return std::distance(parent->subConditions.cbegin(), it);

    Q_ASSERT(false); // should not happen
    return -1;
}
