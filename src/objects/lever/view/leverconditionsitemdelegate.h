/**
 * src/objects/lever/view/leverconditionsitemdelegate.h
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

#ifndef LEVERCONDITIONSITEMDELEGATE_H
#define LEVERCONDITIONSITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QStringListModel>

class LeverConditionsItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit LeverConditionsItemDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

private slots:
    //void onItemClicked();

private:
    QStringListModel conditionsTypeModel;
};

#endif // LEVERCONDITIONSITEMDELEGATE_H
