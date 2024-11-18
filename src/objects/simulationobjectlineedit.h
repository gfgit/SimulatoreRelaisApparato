/**
 * src/objects/simulationobjectlineedit.h
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

#ifndef SIMULATION_OBJECT_LINEEDIT_H
#define SIMULATION_OBJECT_LINEEDIT_H

#include <QWidget>
#include <QStringList>

class QStringListModel;
class QComboBox;
class QLineEdit;
class QCompleter;

class AbstractSimulationObjectModel;
class AbstractSimulationObject;

class ModeManager;

class SimulationObjectLineEdit : public QWidget
{
    Q_OBJECT
public:
    SimulationObjectLineEdit(ModeManager *mgr,
                             const QStringList& types,
                             QWidget *parent = nullptr);

    AbstractSimulationObject *getObject() const
    {
        return mObject;
    }

public slots:
    void setObject(AbstractSimulationObject *newObject);

signals:
    void objectChanged(AbstractSimulationObject *obj);

private slots:
    void setType(int idx);

private:
    ModeManager *modeMgr = nullptr;

    AbstractSimulationObjectModel *mModel = nullptr;
    AbstractSimulationObject *mObject = nullptr;

    QStringListModel *typesModel;
    QComboBox *mTypesCombo = nullptr;

    QLineEdit *mLineEdit = nullptr;
    QCompleter *mCompleter = nullptr;
};

#endif // SIMULATION_OBJECT_LINEEDIT_H
