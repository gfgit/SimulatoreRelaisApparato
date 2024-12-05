/**
 * src/objects/lever/model/levercontactconditionsmodel.h
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

#ifndef LEVERCONTACTCONDITIONSMODEL_H
#define LEVERCONTACTCONDITIONSMODEL_H

#include <QAbstractTableModel>

#include "../../../utils/genericleverutils.h"
#include "../../../utils/enum_desc.h"

class LeverContactConditionsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        TypeCol = 0,
        FromCol,
        ToCol,
        NCols
    };

    explicit LeverContactConditionsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    LeverPositionConditionSet conditions() const;
    void setConditions(const EnumDesc &desc,
                       const LeverPositionConditionSet &newConditions,
                       bool canWarp);

    void instertConditionAt(int row);
    void removeConditionAt(int row);

    void setPositionRange(int min, int max);

    std::pair<int, int> positionRangeFor(const QModelIndex& idx) const;

    const EnumDesc &positionDesc() const;

signals:
    void changed();

private:
    EnumDesc mPositionDesc;
    LeverPositionConditionSet mConditions;

    int mPositionMin = 0;
    int mPositionMax = 0;
    bool mCanWarpAroundZero = false;
};

#endif // LEVERCONTACTCONDITIONSMODEL_H
