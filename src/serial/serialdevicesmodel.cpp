/**
 * src/serial/serialdevicesmodel.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#include "serialdevicesmodel.h"

#include "serialmanager.h"
#include "serialdevice.h"

#include "../views/modemanager.h"

SerialDevicesModel::SerialDevicesModel(SerialManager *mgr)
    : QAbstractTableModel(mgr)
{
}

SerialManager *SerialDevicesModel::serialMgr() const
{
    return static_cast<SerialManager *>(parent());
}

QVariant SerialDevicesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case NameCol:
            return tr("Name");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int SerialDevicesModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mSerialDevices.count();
}

int SerialDevicesModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant SerialDevicesModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mSerialDevices.size())
        return QVariant();

    const SerialDevice *serialDevice = mSerialDevices.at(idx.row());

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (idx.column())
        {
        case NameCol:
            return serialDevice->getName();
        default:
            break;
        }
    }

    return QVariant();
}

bool SerialDevicesModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(serialMgr()->modeMgr()->mode() != FileMode::Editing)
        return false;

    if (!idx.isValid() || idx.row() >= mSerialDevices.size())
        return false;

    SerialDevice *serialDevice = mSerialDevices.at(idx.row());

    if(role == Qt::EditRole)
    {
        switch (idx.column())
        {
        case NameCol:
            return serialDevice->setName(value.toString(), serialMgr());
        default:
            break;
        }
    }

    return false;
}

Qt::ItemFlags SerialDevicesModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mSerialDevices.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(serialMgr()->modeMgr()->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

void SerialDevicesModel::sortItems()
{
    beginResetModel();

    std::sort(mSerialDevices.begin(),
              mSerialDevices.end(),
              [](SerialDevice *a, SerialDevice *b) -> bool
    {
        return a->getName() < b->getName();
    });

    endResetModel();
}

void SerialDevicesModel::clear()
{
    beginResetModel();
    mSerialDevices.clear();
    endResetModel();
}

void SerialDevicesModel::addSerialDevice(SerialDevice *serialDevice)
{
    beginResetModel();

    mSerialDevices.append(serialDevice);

    std::sort(mSerialDevices.begin(),
              mSerialDevices.end(),
              [](SerialDevice *a, SerialDevice *b) -> bool
    {
        return a->getName() < b->getName();
    });

    endResetModel();
}

void SerialDevicesModel::removeSerialDevice(SerialDevice *serialDevice)
{
    const int row = mSerialDevices.indexOf(serialDevice);
    if(row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mSerialDevices.removeAt(row);
    endRemoveRows();
}
