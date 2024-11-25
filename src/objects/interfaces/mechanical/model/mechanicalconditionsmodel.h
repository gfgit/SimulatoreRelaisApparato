/**
 * src/objects/interfaces/mechanical/model/mechanicalconditionsmodel.h
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

#ifndef MECHANICAL_CONDITIONS_MODEL_H
#define MECHANICAL_CONDITIONS_MODEL_H

#include <QAbstractItemModel>

#include "../../../../utils/enum_desc.h"

class MechanicalInterface;
class MechanicalCondition;
class AbstractSimulationObject;

class MechanicalConditionsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column
    {
        ObjectCol = 0,
        TypeCol,
        FromCol,
        ToCol,
        NCols
    };

    struct Item
    {
        typedef std::pair<int, int> LockRange;

        // TODO: make single enum with MechanicalCondition
        enum class Type
        {
            ExactPos = 0,
            RangePos,
            NotPos,
            Or,
            And
        };

        ~Item()
        {
            qDeleteAll(subConditions);
            subConditions.clear();
        }

        Item *parent = nullptr;

        Type type = Type::ExactPos;

        MechanicalInterface *otherIface = nullptr;
        LockRange requiredPositions = {0, 0};

        QVector<Item *> subConditions;

        int row() const;
    };

    explicit MechanicalConditionsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:

    QModelIndex index(int row, int column,
                      const QModelIndex &p = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    MechanicalCondition getConditionTree() const;
    void setConditionTree(const MechanicalCondition &newConditions);

    QModelIndex instertConditionAt(const QModelIndex &idx, Item::Type type);
    void removeConditionAt(const QModelIndex &idx);

    EnumDesc getPositionDescFor(const QModelIndex& idx) const;
    QVector<int> allowedPositionsFor(const QModelIndex& idx) const;
    QVector<int> allowedTypesFor(const QModelIndex& idx) const;

    AbstractSimulationObject *getObjectAt(const QModelIndex& idx) const;
    bool setObjectAt(const QModelIndex& idx, AbstractSimulationObject *obj);

signals:
    void changed();

private:
    void recursiveParseTree(Item *p, const MechanicalCondition& c);
    MechanicalCondition recursiveBuildConditions(const Item *item) const;

    // Return grandparent index
    QModelIndex moveChildrenUpAndRemoveParent(Item *parentItem, const QModelIndex& parentIdx);

private:
    Item rootItem;
};

#endif // MECHANICAL_CONDITIONS_MODEL_H
