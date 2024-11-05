/**
 * src/objects/acei_lever/model/aceilevermodel.cpp
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

#include "aceilevermodel.h"

#include "aceileverobject.h"

#include "../../../views/modemanager.h"

#include <QColor>

#include <QJsonObject>
#include <QJsonArray>

ACEILeverModel::ACEILeverModel(ModeManager *mgr, QObject *parent)
    : QAbstractListModel(parent)
    , mModeMgr(mgr)
{
}

QVariant ACEILeverModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && section == 0 && role == Qt::DisplayRole)
    {
        return tr("Relais");
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

int ACEILeverModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mLevers.size();
}

QVariant ACEILeverModel::data(const QModelIndex &idx, int role) const
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
    //     case ACEILeverObject::State::Up:
    //         color = Qt::red;
    //         break;
    //     case ACEILeverObject::State::GoingUp:
    //     case ACEILeverObject::State::GoingDown:
    //         color.setRgb(120, 210, 255); // Light blue
    //         break;
    //     case ACEILeverObject::State::Down:
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

bool ACEILeverModel::setData(const QModelIndex &idx, const QVariant &value, int role)
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

Qt::ItemFlags ACEILeverModel::flags(const QModelIndex &idx) const
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

void ACEILeverModel::addLever(ACEILeverObject *r)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mLevers.size();
    beginInsertRows(QModelIndex(), row, row);

    connect(r, &QObject::destroyed, this, &ACEILeverModel::onLeverDestroyed);
    connect(r, &ACEILeverObject::nameChanged, this, &ACEILeverModel::onLeverChanged);
    connect(r, &ACEILeverObject::positionChanged, this, &ACEILeverModel::onLeverStateChanged);
    mLevers.append(r);

    endInsertRows();

    onLeverEdited();
}

void ACEILeverModel::removeLever(ACEILeverObject *r)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mLevers.indexOf(r);
    if(row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);

    disconnect(r, &QObject::destroyed, this, &ACEILeverModel::onLeverDestroyed);
    disconnect(r, &ACEILeverObject::nameChanged, this, &ACEILeverModel::onLeverChanged);
    disconnect(r, &ACEILeverObject::positionChanged, this, &ACEILeverModel::onLeverStateChanged);
    mLevers.removeAt(row);

    endRemoveRows();

    onLeverEdited();
}

ACEILeverObject *ACEILeverModel::leverAt(int row) const
{
    return mLevers.value(row, nullptr);
}

ACEILeverObject *ACEILeverModel::getLever(const QString &name)
{
    for(int i = 0; i < mLevers.size(); i++)
    {
        if(mLevers.at(i)->name() == name)
            return mLevers.at(i);
    }
    return nullptr;
}

void ACEILeverModel::clear()
{
    beginResetModel();

    for(ACEILeverObject *r : std::as_const(mLevers))
    {
        disconnect(r, &QObject::destroyed, this, &ACEILeverModel::onLeverDestroyed);
        disconnect(r, &ACEILeverObject::nameChanged, this, &ACEILeverModel::onLeverChanged);
        disconnect(r, &ACEILeverObject::positionChanged, this, &ACEILeverModel::onLeverStateChanged);
        delete r;
    }
    mLevers.clear();

    endResetModel();
}

bool ACEILeverModel::loadFromJSON(const QJsonObject &obj)
{
    beginResetModel();

    const QJsonArray arr = obj.value("relais").toArray();
    for(const QJsonValue& v : arr)
    {
        ACEILeverObject *r = new ACEILeverObject(this);
        r->loadFromJSON(v.toObject());

        connect(r, &QObject::destroyed, this, &ACEILeverModel::onLeverDestroyed);
        connect(r, &ACEILeverObject::nameChanged, this, &ACEILeverModel::onLeverChanged);
        connect(r, &ACEILeverObject::positionChanged, this, &ACEILeverModel::onLeverStateChanged);
        mLevers.append(r);
    }

    endResetModel();
    
    return true;
}

void ACEILeverModel::saveToJSON(QJsonObject &obj) const
{
    QJsonArray arr;

    for(ACEILeverObject *r : std::as_const(mLevers))
    {
        QJsonObject relay;
        r->saveToJSON(relay);
        arr.append(relay);
    }

    obj["relais"] = arr;
}

bool ACEILeverModel::isNameAvailable(const QString &name) const
{
    return std::none_of(mLevers.cbegin(),
                        mLevers.cend(),
                        [name](ACEILeverObject *r) -> bool
    {
        return r->name() == name;
    });
}

void ACEILeverModel::onLeverChanged(ACEILeverObject *r)
{
    updateLeverRow(r);
    onLeverEdited();
}

void ACEILeverModel::onLeverStateChanged(ACEILeverObject *r)
{
    // No changes to save in this case
    updateLeverRow(r);
}

void ACEILeverModel::onLeverDestroyed(QObject *obj)
{
    ACEILeverObject *r = static_cast<ACEILeverObject *>(obj);
    int row = mLevers.indexOf(r);
    Q_ASSERT(row >= 0);

    beginRemoveRows(QModelIndex(), row, row);
    mLevers.removeAt(row);
    endRemoveRows();

    onLeverEdited();
}

void ACEILeverModel::updateLeverRow(ACEILeverObject *r)
{
    int row = mLevers.indexOf(r);
    Q_ASSERT(row >= 0);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);
}

void ACEILeverModel::onLeverEdited()
{
    if(mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = true;
    modeMgr()->setFileEdited();
    emit modelEdited(true);
}

void ACEILeverModel::resetHasUnsavedChanges()
{
    if(!mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = false;
    emit modelEdited(false);
}
