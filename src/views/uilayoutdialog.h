/**
 * src/views/uilayoutdialog.h
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

#ifndef UILAYOUTDIALOG_H
#define UILAYOUTDIALOG_H

#include <QDialog>

class QPushButton;
class QTableView;
class QModelIndex;

class QCheckBox;

class ViewManager;

class UILayoutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UILayoutDialog(ViewManager *viewMgr,
                            QWidget *parent = nullptr);

private slots:
    void onAdd();
    void onRemove();
    void onStoreCurrent();
    void onEditName();
    void onSetLoadAtStart();
    void onActivated(const QModelIndex& idx);

    void onLastLayoutCBChanged();
    void onStartLayoutChanged();

private:
    ViewManager *mViewMgr = nullptr;

    QPushButton *mAddBut;
    QPushButton *mRemBut;
    QPushButton *mEditCurrentBut;
    QPushButton *mStoreCurrentBut;
    QPushButton *mSetLoadAtStart;

    QCheckBox *mLastLayoutAtStartCB;

    QTableView *mView;
};

#endif // UILAYOUTDIALOG_H
