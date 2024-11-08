/**
 * src/objects/lever/model/genericlevermodel.cpp
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

#include "genericlevermodel.h"

#include "genericleverobject.h"

// TODO: factory for different lever types
#include "../acei/aceileverobject.h"

#include "../../../views/modemanager.h"

#include <QColor>

#include <QJsonObject>
#include <QJsonArray>

GenericLeverModel::GenericLeverModel(ModeManager *mgr, QObject *parent)
    : QAbstractListModel(parent)
    , mModeMgr(mgr)
{
}

GenericLeverModel::~GenericLeverModel()
{
    clear();
}

QVariant GenericLeverModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && section == 0 && role == Qt::DisplayRole)
    {
        return tr("Levers");
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

int GenericLeverModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mLevers.size();
}

QVariant GenericLeverModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mLevers.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return mLevers.at(idx.row())->name();
    // case Qt::DecorationRole:
    // {
    //     QColor color = Qt::black;
    //     switch (mLevers.at(idx.row())->state())
    //     {
    //     case GenericLeverObject::State::Up:
    //         color = Qt::red;
    //         break;
    //     case GenericLeverObject::State::GoingUp:
    //     case GenericLeverObject::State::GoingDown:
    //         color.setRgb(120, 210, 255); // Light blue
    //         break;
    //     case GenericLeverObject::State::Down:
    //     default:
    //         break;
    //     }

    //     return color;
    // }
    default:
        break;
    }

    // FIXME: Implement me!
    return QVariant();
}

bool GenericLeverModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return false;

    if (!idx.isValid() || idx.row() >= mLevers.size() || role != Qt::EditRole)
        return false;

    QString name = value.toString().simplified();
    if(name.isEmpty())
        return false;

    if(!isNameAvailable(name))
        return false;

    // Set name
    mLevers.at(idx.row())->setName(name);

    emit dataChanged(idx, idx);
    return true;
}

Qt::ItemFlags GenericLeverModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mLevers.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(mModeMgr->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

void GenericLeverModel::addLever(GenericLeverObject *r)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mLevers.size();
    beginInsertRows(QModelIndex(), row, row);

    connect(r, &QObject::destroyed, this, &GenericLeverModel::onLeverDestroyed);
    connect(r, &GenericLeverObject::nameChanged, this, &GenericLeverModel::onLeverChanged);
    connect(r, &GenericLeverObject::changed, this, &GenericLeverModel::onLeverChanged);
    connect(r, &GenericLeverObject::positionChanged, this, &GenericLeverModel::onLeverStateChanged);
    mLevers.append(r);

    endInsertRows();

    onLeverEdited();
}

void GenericLeverModel::removeLever(GenericLeverObject *r)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mLevers.indexOf(r);
    if(row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);

    disconnect(r, &QObject::destroyed, this, &GenericLeverModel::onLeverDestroyed);
    disconnect(r, &GenericLeverObject::nameChanged, this, &GenericLeverModel::onLeverChanged);
    disconnect(r, &GenericLeverObject::changed, this, &GenericLeverModel::onLeverChanged);
    disconnect(r, &GenericLeverObject::positionChanged, this, &GenericLeverModel::onLeverStateChanged);
    mLevers.removeAt(row);

    endRemoveRows();

    onLeverEdited();
}

GenericLeverObject *GenericLeverModel::leverAt(int row) const
{
    return mLevers.value(row, nullptr);
}

GenericLeverObject *GenericLeverModel::getLever(const QString &name)
{
    for(int i = 0; i < mLevers.size(); i++)
    {
        if(mLevers.at(i)->name() == name)
            return mLevers.at(i);
    }
    return nullptr;
}

void GenericLeverModel::clear()
{
    beginResetModel();

    clearInternal();

    endResetModel();
}

bool GenericLeverModel::loadFromJSON(const QJsonObject &obj)
{
    beginResetModel();

    clearInternal();

    const QJsonArray arr = obj.value("levers").toArray();
    for(const QJsonValue& v : arr)
    {
        QJsonObject leverObj = v.toObject();
        if(getLever(leverObj.value("name").toString()))
            continue; // Skip duplicates

        GenericLeverObject *l = new ACEILeverObject(this);
        l->loadFromJSON(leverObj);

        connect(l, &QObject::destroyed, this, &GenericLeverModel::onLeverDestroyed);
        connect(l, &GenericLeverObject::nameChanged, this, &GenericLeverModel::onLeverChanged);
        connect(l, &GenericLeverObject::changed, this, &GenericLeverModel::onLeverChanged);
        connect(l, &GenericLeverObject::positionChanged, this, &GenericLeverModel::onLeverStateChanged);
        mLevers.append(l);
    }

    std::sort(mLevers.begin(),
              mLevers.end(),
              [](const GenericLeverObject *lhs, const GenericLeverObject *rhs) -> bool
    {
        return lhs->name() < rhs->name();
    });

    endResetModel();
    
    return true;
}

void GenericLeverModel::saveToJSON(QJsonObject &obj) const
{
    QJsonArray arr;

    for(GenericLeverObject *l : std::as_const(mLevers))
    {
        QJsonObject lever;
        l->saveToJSON(lever);
        arr.append(lever);
    }

    obj["levers"] = arr;
}

bool GenericLeverModel::isNameAvailable(const QString &name) const
{
    return std::none_of(mLevers.cbegin(),
                        mLevers.cend(),
                        [name](GenericLeverObject *r) -> bool
    {
        return r->name() == name;
    });
}

void GenericLeverModel::onLeverChanged(GenericLeverObject *r)
{
    updateLeverRow(r);
    onLeverEdited();
}

void GenericLeverModel::onLeverStateChanged(GenericLeverObject *r)
{
    // No changes to save in this case
    updateLeverRow(r);
}

void GenericLeverModel::onLeverDestroyed(QObject *obj)
{
    GenericLeverObject *r = static_cast<GenericLeverObject *>(obj);
    int row = mLevers.indexOf(r);
    Q_ASSERT(row >= 0);

    beginRemoveRows(QModelIndex(), row, row);
    mLevers.removeAt(row);
    endRemoveRows();

    onLeverEdited();
}

void GenericLeverModel::updateLeverRow(GenericLeverObject *r)
{
    int row = mLevers.indexOf(r);
    Q_ASSERT(row >= 0);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);
}

void GenericLeverModel::onLeverEdited()
{
    if(mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = true;
    modeMgr()->setFileEdited();
    emit modelEdited(true);
}

void GenericLeverModel::clearInternal()
{
    for(GenericLeverObject *r : std::as_const(mLevers))
    {
        disconnect(r, &QObject::destroyed, this, &GenericLeverModel::onLeverDestroyed);
        disconnect(r, &GenericLeverObject::nameChanged, this, &GenericLeverModel::onLeverChanged);
        disconnect(r, &GenericLeverObject::changed, this, &GenericLeverModel::onLeverChanged);
        disconnect(r, &GenericLeverObject::positionChanged, this, &GenericLeverModel::onLeverStateChanged);
        delete r;
    }
    mLevers.clear();
}

void GenericLeverModel::resetHasUnsavedChanges()
{
    if(!mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = false;
    emit modelEdited(false);
}
