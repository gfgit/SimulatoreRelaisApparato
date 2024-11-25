/**
 * src/objects/interfaces/mechanical/view/mechanicalconditionsview.h
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

#ifndef MECHANICAL_CONDITIONS_VIEW_H
#define MECHANICAL_CONDITIONS_VIEW_H

#include <QWidget>

class MechanicalConditionsModel;

class QPushButton;
class QTreeView;

class ModeManager;

class MechanicalConditionsView : public QWidget
{
    Q_OBJECT
public:
    explicit MechanicalConditionsView(ModeManager *mgr, QWidget *parent = nullptr);

    MechanicalConditionsModel *model() const;
    void setModel(MechanicalConditionsModel *newModel);

    void expandAll();

private slots:
    void addCondition();
    void removeCurrentCondition();

private:
    QTreeView *mView;

    QPushButton *addBut;
    QPushButton *remBut;

    QPushButton *addOrBut;
    QPushButton *addAndBut;

    MechanicalConditionsModel *mModel = nullptr;
};

#endif // MECHANICAL_CONDITIONS_VIEW_H
