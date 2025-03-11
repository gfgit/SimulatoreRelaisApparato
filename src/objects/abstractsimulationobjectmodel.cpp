/**
 * src/objects/abstractsimulationobjectmodel.cpp
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

#include "abstractsimulationobjectmodel.h"
#include "abstractsimulationobject.h"

#include "simulationobjectfactory.h"

#include "../views/modemanager.h"

#include "../enums/loadphase.h"

#include <QJsonObject>
#include <QJsonArray>

#include <QFont>
#include <QColor>


AbstractSimulationObjectModel::AbstractSimulationObjectModel(ModeManager *mgr,
                                                             const QString &objTypeName,
                                                             QObject *parent)
    : QAbstractTableModel(parent)
    , mModeMgr(mgr)
    , mObjectType(objTypeName)

{

}

AbstractSimulationObjectModel::~AbstractSimulationObjectModel()
{
    clear();
}

QVariant AbstractSimulationObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case NameCol:
            return tr("Name");
        case NodesCol:
            return tr("Nodes");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}


int AbstractSimulationObjectModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mObjects.size();
}

int AbstractSimulationObjectModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant AbstractSimulationObjectModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mObjects.size() || idx.column() >= NCols)
        return QVariant();

    AbstractSimulationObject *item = mObjects.at(idx.row());

    switch (idx.column())
    {
    case NameCol:
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return item->name();
        default:
            break;
        }
        break;
    }
    case NodesCol:
    {
        int count = item->getReferencingNodes(nullptr);
        return nodesCountData(item, role,
                              count, count == 0);
    }
    default:
        break;
    }

    return QVariant();
}

bool AbstractSimulationObjectModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return false;

    if (!idx.isValid() || idx.row() >= mObjects.size() || role != Qt::EditRole)
        return false;

    QString name = value.toString().simplified();
    if(name.isEmpty())
        return false;

    // Set name
    if(!mObjects.at(idx.row())->setName(name))
        return false;

    emit dataChanged(idx, idx);
    return true;
}

Qt::ItemFlags AbstractSimulationObjectModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mObjects.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(mModeMgr->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

AbstractSimulationObject *AbstractSimulationObjectModel::objectAt(int row) const
{
    return mObjects.value(row, nullptr);
}

AbstractSimulationObject *AbstractSimulationObjectModel::getObjectByName(const QString &name) const
{
    for(int i = 0; i < mObjects.size(); i++)
    {
        if(mObjects.at(i)->name() == name)
            return mObjects.at(i);
    }
    return nullptr;
}

QString AbstractSimulationObjectModel::getObjectPrettyName() const
{
    return modeMgr()->objectFactory()->prettyName(mObjectType);
}

void AbstractSimulationObjectModel::clear()
{
    beginResetModel();

    clearInternal();

    endResetModel();
}

bool AbstractSimulationObjectModel::loadFromJSON(const QJsonObject &modelObj, LoadPhase phase)
{
    if(modelObj.value("model_type") != mObjectType)
        return false;

    if(phase == LoadPhase::Creation)
    {
        beginResetModel();

        clearInternal();
    }

    const QJsonArray arr = modelObj.value("objects").toArray();

    QJsonArray result;
    addObjectsFromArray_internal(arr, result, phase);

    if(phase == LoadPhase::Creation)
    {
        std::sort(mObjects.begin(),
                  mObjects.end(),
                  [](const AbstractSimulationObject *a,
                  const AbstractSimulationObject *b) -> bool
        {
            return a->name() < b->name();
        });

        endResetModel();
    }

    return true;
}

void AbstractSimulationObjectModel::saveToJSON(QJsonObject &modelObj) const
{
    QJsonArray arr;

    for(AbstractSimulationObject *item : std::as_const(mObjects))
    {
        QJsonObject obj;
        item->saveToJSON(obj);
        arr.append(obj);
    }

    modelObj["model_type"] = mObjectType;
    modelObj["objects"] = arr;
}

void AbstractSimulationObjectModel::addObject(AbstractSimulationObject *item)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mObjects.size();
    beginInsertRows(QModelIndex(), row, row);

    addObjectInternal(item);
    mObjects.append(item);

    endInsertRows();

    setModelEdited();
}

void AbstractSimulationObjectModel::removeObject(AbstractSimulationObject *item)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mObjects.indexOf(item);
    if(row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);

    removeObjectInternal(item);
    mObjects.removeAt(row);
    delete item;

    endRemoveRows();

    setModelEdited();
}

void AbstractSimulationObjectModel::addObjectsFromArray(const QJsonArray &arr,
                                                        QJsonArray &result,
                                                        LoadPhase phase)
{
    if(phase == LoadPhase::Creation)
    {
        beginResetModel();
    }

    addObjectsFromArray_internal(arr, result, phase);

    if(phase == LoadPhase::Creation)
    {
        endResetModel();
    }
}

bool AbstractSimulationObjectModel::isNameAvailable(const QString &name) const
{
    return getObjectByName(name) == nullptr;
}

void AbstractSimulationObjectModel::resetHasUnsavedChanges()
{
    if(!mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = false;
    emit modelEdited(false);
}

void AbstractSimulationObjectModel::onObjectChanged(AbstractSimulationObject *item)
{
    updateObjectRow(item);
    setModelEdited();
}

void AbstractSimulationObjectModel::onObjectStateChanged(AbstractSimulationObject *item)
{
    // No changes to save in this case
    updateObjectRow(item);
}

void AbstractSimulationObjectModel::onObjectDestroyed(QObject *obj)
{
    AbstractSimulationObject *item = static_cast<AbstractSimulationObject *>(obj);
    int row = mObjects.indexOf(item);
    Q_ASSERT(row >= 0);

    beginRemoveRows(QModelIndex(), row, row);
    mObjects.removeAt(row);
    endRemoveRows();

    setModelEdited();
}

void AbstractSimulationObjectModel::updateObjectRow(AbstractSimulationObject *item)
{
    int row = mObjects.indexOf(item);
    Q_ASSERT(row >= 0);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx.siblingAtColumn(columnCount() - 1));
}

void AbstractSimulationObjectModel::setModelEdited()
{
    if(mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = true;
    modeMgr()->setFileEdited();
    emit modelEdited(true);
}

void AbstractSimulationObjectModel::clearInternal()
{
    for(AbstractSimulationObject *item : std::as_const(mObjects))
    {
        removeObjectInternal(item);
        delete item;
    }
    mObjects.clear();
}

void AbstractSimulationObjectModel::addObjectsFromArray_internal(const QJsonArray &arr,
                                                                 QJsonArray &result,
                                                                 LoadPhase phase)
{
    if(phase == LoadPhase::Creation)
    {
        // Create all objects
        SimulationObjectFactory *factory = modeMgr()->objectFactory();
        for(const QJsonValue& v : arr)
        {
            QJsonObject obj = v.toObject();
            if(getObjectByName(obj.value("name").toString()))
                continue; // Skip duplicates

            AbstractSimulationObject *item = factory->createItem(this);
            if(!item->loadFromJSON(obj, LoadPhase::Creation))
            {
                delete item;
                continue;
            }

            result.append(v);

            addObjectInternal(item);
            mObjects.append(item);
        }
    }
    else
    {
        // Now that all objects are created,
        // let objects connect to each other

        QSet<QString> loadedObjs;

        for(const QJsonValue& v : arr)
        {
            QJsonObject obj = v.toObject();
            const QString name = obj.value("name").toString();
            if(loadedObjs.contains(name))
                continue; // Duplicate

            AbstractSimulationObject *item = getObjectByName(name);

            // In this second phase we ignore the result
            item->loadFromJSON(obj, LoadPhase::AllCreated);

            loadedObjs.insert(name);
        }
    }
}

void AbstractSimulationObjectModel::addObjectInternal(AbstractSimulationObject *item)
{
    connect(item, &QObject::destroyed,
            this, &AbstractSimulationObjectModel::onObjectDestroyed);
    connect(item, &AbstractSimulationObject::settingsChanged,
            this, &AbstractSimulationObjectModel::onObjectChanged);
    connect(item, &AbstractSimulationObject::stateChanged,
            this, &AbstractSimulationObjectModel::onObjectStateChanged);
    connect(item, &AbstractSimulationObject::nodesChanged,
            this, &AbstractSimulationObjectModel::onObjectStateChanged);
}

void AbstractSimulationObjectModel::removeObjectInternal(AbstractSimulationObject *item)
{
    disconnect(item, &QObject::destroyed,
               this, &AbstractSimulationObjectModel::onObjectDestroyed);
    disconnect(item, &AbstractSimulationObject::settingsChanged,
               this, &AbstractSimulationObjectModel::onObjectChanged);
    disconnect(item, &AbstractSimulationObject::stateChanged,
               this, &AbstractSimulationObjectModel::onObjectStateChanged);
    disconnect(item, &AbstractSimulationObject::nodesChanged,
               this, &AbstractSimulationObjectModel::onObjectStateChanged);
}

QVariant AbstractSimulationObjectModel::nodesCountData(const AbstractSimulationObject *item,
                                                       int role,
                                                       int nodesCount, bool highlight,
                                                       const QString &tip) const
{
    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return nodesCount;
    case Qt::TextAlignmentRole:
        return int(Qt::AlignRight | Qt::AlignVCenter);
    case Qt::FontRole:
    {
        if(highlight)
        {
            // Highlight objects not referenced by nodes
            // Bold font
            QFont f;
            f.setBold(true);
            return f;
        }

        break;
    }
    case Qt::BackgroundRole:
    {
        if(highlight)
        {
            // Highlight objects not referenced by nodes
            // Light red background
            return QColor(qRgb(255, 140, 140));
        }

        break;
    }
    case Qt::ToolTipRole:
    {
        if(!tip.isNull())
            return tip;

        return tr("Object <b>%1</b> is referenced in <b>%2</b> nodes.")
                .arg(item->name()).arg(nodesCount);
    }
    default:
        break;
    }

    return QVariant();
}
