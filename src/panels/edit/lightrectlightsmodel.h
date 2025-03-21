/**
 * src/panels/edit/lightrectlightsmodel.h
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

#ifndef LIGHTRECTLIGHTSMODEL_H
#define LIGHTRECTLIGHTSMODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include <QVector>

class LightBulbObject;

class LightRectLightsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    struct LightEntry
    {
        LightBulbObject *light = nullptr;
        QColor color = Qt::red;
    };

    enum Columns
    {
        LightCol = 0,
        ColorCol,
        NCols
    };

    explicit LightRectLightsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    LightEntry getEntryAt(int row) const
    {
        return mItems.value(row, {});
    }

    void setEntryAt(int row, const LightEntry& entry);

    void removeEntryAt(int row);
    void addEntryAt(int row, const LightEntry &entry);
    void moveRow(int row, bool up);

    inline bool isChanged() const
    {
        return changed;
    }

    inline void resetChanged()
    {
        changed = false;
    }

    QVector<LightEntry> items() const;
    void setItems(const QVector<LightEntry> &newItems);

signals:
    void needsSave();

private:
    void setChanged()
    {
        if(changed)
            return;

        changed = true;
        emit needsSave();
    }

    QVector<LightEntry> mItems;
    bool changed = false;
};

inline bool operator==(const LightRectLightsModel::LightEntry& lhs, const LightRectLightsModel::LightEntry& rhs)
{
    return lhs.light == rhs.light && lhs.color == rhs.color;
}

#endif // LIGHTRECTLIGHTSMODEL_H
