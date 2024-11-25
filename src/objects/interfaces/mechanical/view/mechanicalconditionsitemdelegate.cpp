/**
 * src/objects/interfaces/mechanical/view/mechanicalconditionsitemdelegate.cpp
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

#include "mechanicalconditionsitemdelegate.h"

#include "../../mechanicalinterface.h"

#include "../../../../utils/enumvaluesmodel.h"
#include "../model/mechanicalconditionsmodel.h"

#include "../../../../views/modemanager.h"
#include "../../../simulationobjectfactory.h"

#include "../../../simulationobjectlineedit.h"
#include <QComboBox>

MechanicalConditionsItemDelegate::MechanicalConditionsItemDelegate(ModeManager *mgr, QObject *parent)
    : QStyledItemDelegate{parent}
    , modeMgr(mgr)
{

}

QWidget *MechanicalConditionsItemDelegate::createEditor(QWidget *parent,
                                                        const QStyleOptionViewItem &options,
                                                        const QModelIndex &index) const
{
    const MechanicalConditionsModel *sourceModel =
            static_cast<const MechanicalConditionsModel *>(index.model());

    if(index.column() == MechanicalConditionsModel::ObjectCol)
    {
        QStringList objTypes = modeMgr->objectFactory()
                ->typesForInterface(MechanicalInterface::IfaceType);
        SimulationObjectLineEdit *objEdit
                = new SimulationObjectLineEdit(modeMgr, objTypes, parent);
        return objEdit;
    }
    else if(index.column() == MechanicalConditionsModel::TypeCol ||
            index.column() == MechanicalConditionsModel::FromCol ||
            index.column() == MechanicalConditionsModel::ToCol)
    {
        QComboBox *combo = new QComboBox(parent);
        EnumValuesModel *enumValuesModel = new EnumValuesModel;

        if(index.column() == MechanicalConditionsModel::TypeCol)
        {
            enumValuesModel->setEnumDescFiltered(MechanicalCondition::getTypeDesc(),
                                                 sourceModel->allowedTypesFor(index));
        }
        else
        {
            enumValuesModel->setEnumDescFiltered(sourceModel->getPositionDescFor(index),
                                                 sourceModel->allowedPositionsFor(index));
        }

        combo->setModel(enumValuesModel);
        return combo;
    }

    return nullptr;
}

void MechanicalConditionsItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const MechanicalConditionsModel *sourceModel =
            static_cast<const MechanicalConditionsModel *>(index.model());

    if(index.column() == MechanicalConditionsModel::ObjectCol)
    {
        SimulationObjectLineEdit *objEdit = static_cast<SimulationObjectLineEdit *>(editor);
        objEdit->setObject(sourceModel->getObjectAt(index));
    }
    else if(index.column() == MechanicalConditionsModel::TypeCol ||
            index.column() == MechanicalConditionsModel::FromCol ||
            index.column() == MechanicalConditionsModel::ToCol)
    {
        QComboBox *combo = static_cast<QComboBox *>(editor);
        const int value = index.data(Qt::EditRole).toInt();

        EnumValuesModel *enumValuesModel =
                static_cast<EnumValuesModel *>(combo->model());
        combo->setCurrentIndex(enumValuesModel->rowForValue(value));

        // connect(combo, &QComboBox::activated,
        //         this, &MechanicalConditionsItemDelegate::onItemClicked,
        //         Qt::UniqueConnection);

        //combo->showPopup();
    }
}

void MechanicalConditionsItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    MechanicalConditionsModel *sourceModel =
            static_cast<MechanicalConditionsModel *>(model);

    if(index.column() == MechanicalConditionsModel::ObjectCol)
    {
        SimulationObjectLineEdit *objEdit = static_cast<SimulationObjectLineEdit *>(editor);
        sourceModel->setObjectAt(index, objEdit->getObject());
    }
    else if(index.column() == MechanicalConditionsModel::TypeCol ||
            index.column() == MechanicalConditionsModel::FromCol ||
            index.column() == MechanicalConditionsModel::ToCol)
    {
        QComboBox *combo = static_cast<QComboBox *>(editor);

        EnumValuesModel *enumValuesModel =
                static_cast<EnumValuesModel *>(combo->model());
        const int value = enumValuesModel->valueAt(combo->currentIndex());

        model->setData(index, value, Qt::EditRole);
    }
}

// void MechanicalConditionsItemDelegate::onItemClicked()
// {
//     QComboBox *combo = qobject_cast<QComboBox *>(sender());
//     if (combo)
//     {
//         emit commitData(combo);
//         emit closeEditor(combo);
//     }
// }
