/**
 * src/objects/simulationobjectoptionswidget.h
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

#ifndef SIMULATIONOBJECTOPTIONSWIDGET_H
#define SIMULATIONOBJECTOPTIONSWIDGET_H

#include <QWidget>

class QLineEdit;
class QTabWidget;
class QScrollArea;
class QTableView;

class AbstractSimulationObjectModel;
class AbstractSimulationObject;

class SimulationObjectNodesModel;

class ViewManager;

class SimulationObjectOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimulationObjectOptionsWidget(AbstractSimulationObject *object,
                                           ViewManager *viewMgr,
                                           QWidget *parent = nullptr);

    void addCustomWidget(QWidget *w);

    QString uniqueId() const;

    void fixScrollingChildrenInScrollArea();

    bool eventFilter(QObject *watched, QEvent *ev) override;

    void setEditingAllowed(bool value);

signals:
    void uniqueIdChanged(const QString& uniqueId);

private slots:
    void setName();
    void onNameTextEdited();
    void onNameChanged();

    void onNodeClicked(const QModelIndex& idx);

private:
    void setNameValid(bool valid);

private:
    ViewManager *mViewMgr = nullptr;
    AbstractSimulationObject *mObject = nullptr;

    QLineEdit *mNameEdit = nullptr;
    QLineEdit *mDescriptionEdit = nullptr;

    QPalette normalEditPalette;

    QTabWidget *tabWidget = nullptr;
    QScrollArea *scrollArea = nullptr;

    QTableView *mNodesView = nullptr;
    SimulationObjectNodesModel *mNodesModel = nullptr;
};

#endif // SIMULATIONOBJECTOPTIONSWIDGET_H
