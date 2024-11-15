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

#include <QJsonObject>
#include <QJsonArray>


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
    // FIXME: Implement me!
    return QAbstractTableModel::headerData(section, orientation, role);
}


int AbstractSimulationObjectModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mObjects.size();
}

int AbstractSimulationObjectModel::columnCount(const QModelIndex &p) const
{
    if (p.isValid())
        return 0;

    // FIXME: Implement me!
    return 1;
}

QVariant AbstractSimulationObjectModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mObjects.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return mObjects.at(idx.row())->name();
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

bool AbstractSimulationObjectModel::loadFromJSON(const QJsonObject &modelObj)
{
    if(modelObj.value("model_type") != mObjectType)
        return false;

    beginResetModel();

    clearInternal();

    SimulationObjectFactory *factory = modeMgr()->objectFactory();

    const QJsonArray arr = modelObj.value("objects").toArray();
    for(const QJsonValue& v : arr)
    {
        QJsonObject obj = v.toObject();
        if(getObjectByName(obj.value("name").toString()))
            continue; // Skip duplicates

        AbstractSimulationObject *item = factory->createItem(this);
        item->loadFromJSON(obj);

        addObjectInternal(item);
        mObjects.append(item);
    }

    std::sort(mObjects.begin(),
              mObjects.end(),
              [](const AbstractSimulationObject *a,
              const AbstractSimulationObject *b) -> bool
    {
        return a->name() < b->name();
    });

    endResetModel();

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

    addObjectInternal(item);
    mObjects.removeAt(row);

    endRemoveRows();

    setModelEdited();
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
    emit dataChanged(idx, idx);
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

void AbstractSimulationObjectModel::addObjectInternal(AbstractSimulationObject *item)
{
    connect(item, &QObject::destroyed,
            this, &AbstractSimulationObjectModel::onObjectDestroyed);
    connect(item, &AbstractSimulationObject::settingsChanged,
            this, &AbstractSimulationObjectModel::onObjectChanged);
    connect(item, &AbstractSimulationObject::stateChanged,
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
}
