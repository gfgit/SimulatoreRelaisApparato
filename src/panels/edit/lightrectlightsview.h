/**
 * src/panels/edit/lightrectlightsview.h
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

#ifndef LIGHTRECTLIGHTSVIEW_H
#define LIGHTRECTLIGHTSVIEW_H

#include <QWidget>

class QPushButton;
class QTableView;

class LightRectLightsModel;
class LightRectItem;

class LightRectLightsView : public QWidget
{
    Q_OBJECT
public:
    explicit LightRectLightsView(QWidget *parent = nullptr);

    void loadFrom(LightRectItem *item);
    void saveTo(LightRectItem *item);

signals:
    void needsSave();

private slots:
    void onAdd();
    void onRemove();
    void onEdit();
    void editIndex(const QModelIndex& idx);
    void onMove(bool up);

private:
    LightRectLightsModel *mModel = nullptr;

    QTableView *mView;
    QPushButton *mAddLightBut;
    QPushButton *mRemLightBut;
    QPushButton *mEditLightBut;
    QPushButton *mUpLightBut;
    QPushButton *mDownLightBut;
};

#endif // LIGHTRECTLIGHTSVIEW_H
