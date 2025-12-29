/**
 * src/objects/traintastic/edit/signalindicatorlistview.h
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

#ifndef SIGNAL_INDICATOR_LIST_VIEW_H
#define SIGNAL_INDICATOR_LIST_VIEW_H

#include <QWidget>

class QPushButton;
class QTableView;

class SignalIndicatorListModel;
class TraintasticSignalObject;

class SignalIndicatorListView : public QWidget
{
    Q_OBJECT
public:
    explicit SignalIndicatorListView(QWidget *parent = nullptr);

    void loadFrom(TraintasticSignalObject *item);
    void saveTo(TraintasticSignalObject *item);

signals:
    void needsSave();

private slots:
    void onAdd();
    void onRemove();
    void onEdit();
    void editIndex(const QModelIndex& idx);
    void onMove(bool up);

private:
    SignalIndicatorListModel *mModel = nullptr;

    QTableView *mView;
    QPushButton *mAddLightBut;
    QPushButton *mRemLightBut;
    QPushButton *mEditLightBut;
    QPushButton *mUpLightBut;
    QPushButton *mDownLightBut;
};

#endif // SIGNAL_INDICATOR_LIST_VIEW_H
