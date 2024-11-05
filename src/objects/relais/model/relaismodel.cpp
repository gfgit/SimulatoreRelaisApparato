/**
 * src/objects/relais/model/relaismodel.cpp
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

#include "relaismodel.h"

#include "abstractrelais.h"

#include "../../../views/modemanager.h"

#include <QColor>

#include <QJsonObject>
#include <QJsonArray>

RelaisModel::RelaisModel(ModeManager *mgr, QObject *parent)
    : QAbstractListModel(parent)
    , mModeMgr(mgr)
{
}

QVariant RelaisModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && section == 0 && role == Qt::DisplayRole)
    {
        return tr("Relais");
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

int RelaisModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mRelais.size();
}

QVariant RelaisModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mRelais.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return mRelais.at(idx.row())->name();
    case Qt::DecorationRole:
    {
        QColor color = Qt::black;
        switch (mRelais.at(idx.row())->state())
        {
        case AbstractRelais::State::Up:
            color = Qt::red;
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color.setRgb(120, 210, 255); // Light blue
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }

        return color;
    }
    default:
        break;
    }

    // FIXME: Implement me!
    return QVariant();
}

bool RelaisModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return false;

    if (!idx.isValid() || idx.row() >= mRelais.size() || role != Qt::EditRole)
        return false;

    QString name = value.toString().simplified();
    if(name.isEmpty())
        return false;

    if(!isNameAvailable(name))
        return false;

    // Set name
    mRelais.at(idx.row())->setName(name);

    emit dataChanged(idx, idx);
    return true;
}

Qt::ItemFlags RelaisModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mRelais.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(mModeMgr->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

void RelaisModel::addRelay(AbstractRelais *r)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mRelais.size();
    beginInsertRows(QModelIndex(), row, row);

    connect(r, &QObject::destroyed, this, &RelaisModel::onRelayDestroyed);
    connect(r, &AbstractRelais::nameChanged, this, &RelaisModel::onRelayChanged);
    connect(r, &AbstractRelais::stateChanged, this, &RelaisModel::onRelayStateChanged);
    mRelais.append(r);

    endInsertRows();

    onRelayEdited();
}

void RelaisModel::removeRelay(AbstractRelais *r)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    int row = mRelais.indexOf(r);
    if(row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);

    disconnect(r, &QObject::destroyed, this, &RelaisModel::onRelayDestroyed);
    disconnect(r, &AbstractRelais::nameChanged, this, &RelaisModel::onRelayChanged);
    disconnect(r, &AbstractRelais::stateChanged, this, &RelaisModel::onRelayStateChanged);
    mRelais.removeAt(row);

    endRemoveRows();

    onRelayEdited();
}

AbstractRelais *RelaisModel::relayAt(int row) const
{
    return mRelais.value(row, nullptr);
}

AbstractRelais *RelaisModel::getRelay(const QString &name)
{
    for(int i = 0; i < mRelais.size(); i++)
    {
        if(mRelais.at(i)->name() == name)
            return mRelais.at(i);
    }
    return nullptr;
}

void RelaisModel::clear()
{
    beginResetModel();

    for(AbstractRelais *r : std::as_const(mRelais))
    {
        disconnect(r, &QObject::destroyed, this, &RelaisModel::onRelayDestroyed);
        disconnect(r, &AbstractRelais::nameChanged, this, &RelaisModel::onRelayChanged);
        disconnect(r, &AbstractRelais::stateChanged, this, &RelaisModel::onRelayStateChanged);
        delete r;
    }
    mRelais.clear();

    endResetModel();
}

bool RelaisModel::loadFromJSON(const QJsonObject &obj)
{
    beginResetModel();

    const QJsonArray arr = obj.value("relais").toArray();
    for(const QJsonValue& v : arr)
    {
        AbstractRelais *r = new AbstractRelais(this);
        r->loadFromJSON(v.toObject());

        connect(r, &QObject::destroyed, this, &RelaisModel::onRelayDestroyed);
        connect(r, &AbstractRelais::nameChanged, this, &RelaisModel::onRelayChanged);
        connect(r, &AbstractRelais::stateChanged, this, &RelaisModel::onRelayStateChanged);
        mRelais.append(r);
    }

    endResetModel();
    
    return true;
}

void RelaisModel::saveToJSON(QJsonObject &obj) const
{
    QJsonArray arr;

    for(AbstractRelais *r : std::as_const(mRelais))
    {
        QJsonObject relay;
        r->saveToJSON(relay);
        arr.append(relay);
    }

    obj["relais"] = arr;
}

bool RelaisModel::isNameAvailable(const QString &name) const
{
    return std::none_of(mRelais.cbegin(),
                        mRelais.cend(),
                        [name](AbstractRelais *r) -> bool
    {
        return r->name() == name;
    });
}

void RelaisModel::onRelayChanged(AbstractRelais *r)
{
    updateRelayRow(r);
    onRelayEdited();
}

void RelaisModel::onRelayStateChanged(AbstractRelais *r)
{
    // No changes to save in this case
    updateRelayRow(r);
}

void RelaisModel::onRelayDestroyed(QObject *obj)
{
    AbstractRelais *r = static_cast<AbstractRelais *>(obj);
    int row = mRelais.indexOf(r);
    Q_ASSERT(row >= 0);

    beginRemoveRows(QModelIndex(), row, row);
    mRelais.removeAt(row);
    endRemoveRows();

    onRelayEdited();
}

void RelaisModel::updateRelayRow(AbstractRelais *r)
{
    int row = mRelais.indexOf(r);
    Q_ASSERT(row >= 0);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);
}

void RelaisModel::onRelayEdited()
{
    if(mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = true;
    modeMgr()->setFileEdited();
    emit modelEdited(true);
}

void RelaisModel::resetHasUnsavedChanges()
{
    if(!mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = false;
    emit modelEdited(false);
}