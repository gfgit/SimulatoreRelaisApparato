/**
 * src/utils/enumvaluesmodel.h
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

#ifndef ENUM_VALUES_MODEL_H
#define ENUM_VALUES_MODEL_H

#include <QAbstractListModel>

#include "enum_desc.h"

class EnumValuesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    EnumValuesModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx,
                  int role = Qt::DisplayRole) const override;

    int valueAt(int row) const;
    int rowForValue(int value) const;

    EnumDesc enumDesc() const;

    void setEnumDescFull(const EnumDesc &newEnumDesc,
                         bool skipMiddleValues);
    void setEnumDescRange(const EnumDesc &newEnumDesc,
                          bool skipMiddleValues,
                          int minVal, int maxVal);
    void setEnumDescFiltered(const EnumDesc &newEnumDesc,
                             const QVector<int>& values);

private:
    EnumDesc mEnumDesc;
    QVector<int> mValues;
};

#endif // ENUM_VALUES_MODEL_H
