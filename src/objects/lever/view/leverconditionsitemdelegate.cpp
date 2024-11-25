/**
 * src/objects/lever/view/leverconditionsitemdelegate.cpp
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

#include "leverconditionsitemdelegate.h"

#include "../../../utils/enumvaluesmodel.h"
#include "../model/levercontactconditionsmodel.h"

#include <QComboBox>

LeverConditionsItemDelegate::LeverConditionsItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
    // Init conditions type model
    QStringList list;
    list.reserve(int(LeverPositionConditionType::NTypes));
    for (int i = 0; i < int(LeverPositionConditionType::NTypes); i++)
        list.append(GenericLeverUtils::getTypeName(LeverPositionConditionType(i)));

    conditionsTypeModel.setStringList(list);
}

QWidget *LeverConditionsItemDelegate::createEditor(QWidget *parent,
                                                   const QStyleOptionViewItem &options,
                                                   const QModelIndex &index) const
{
    QComboBox *combo = new QComboBox(parent);

    if(index.column() == LeverContactConditionsModel::TypeCol)
        combo->setModel(const_cast<QStringListModel *>(&conditionsTypeModel));
    else
    {
        // Get min max range
        const LeverContactConditionsModel *sourceModel =
                static_cast<const LeverContactConditionsModel *>(index.model());

        const auto range = sourceModel->positionRangeFor(index);

        EnumValuesModel *positionModel = new EnumValuesModel(combo);
        positionModel->setEnumDescRange(sourceModel->positionDesc(), true,
                                        range.first, range.second);
        combo->setModel(positionModel);
    }

    return combo;
}

void LeverConditionsItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox *>(editor);
    const int value = index.data(Qt::EditRole).toInt();

    if(index.column() == LeverContactConditionsModel::TypeCol)
        combo->setCurrentIndex(value);
    else
    {
        EnumValuesModel *positionModel =
                static_cast<EnumValuesModel *>(combo->model());
        combo->setCurrentIndex(positionModel->rowForValue(value));
    }

    // connect(combo, &QComboBox::activated,
    //         this, &LeverConditionsItemDelegate::onItemClicked,
    //         Qt::UniqueConnection);

    //combo->showPopup();
}

void LeverConditionsItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox *>(editor);
    int value = combo->currentIndex();

    if(index.column() != LeverContactConditionsModel::TypeCol)
    {
        EnumValuesModel *positionModel =
                static_cast<EnumValuesModel *>(combo->model());
        value = positionModel->valueAt(value);
    }

    model->setData(index, value, Qt::EditRole);
}

// void LeverConditionsItemDelegate::onItemClicked()
// {
//     QComboBox *combo = qobject_cast<QComboBox *>(sender());
//     if (combo)
//     {
//         emit commitData(combo);
//         emit closeEditor(combo);
//     }
// }
