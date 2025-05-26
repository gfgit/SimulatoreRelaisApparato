/**
 * src/objects/simulationobjectfactory.h
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

#ifndef SIMULATIONOBJECTFACTORY_H
#define SIMULATIONOBJECTFACTORY_H

#include <QString>
#include <QVector>

class AbstractSimulationObjectModel;
class AbstractSimulationObject;

class SimulationObjectOptionsWidget;

class ModeManager;
class ViewManager;

class QWidget;

class SimulationObjectFactory
{
public:
    typedef AbstractSimulationObjectModel *(*CreateModelFunc)(ModeManager *);
    typedef AbstractSimulationObject *(*CreateObjectFunc)(AbstractSimulationObjectModel *);
    typedef QWidget*(*EditFunc)(AbstractSimulationObject *, ViewManager *);

    SimulationObjectFactory();

    struct FactoryItem
    {
        QString objectType;
        QString prettyName;
        CreateModelFunc customModelFunc = nullptr;
        CreateObjectFunc create = nullptr;
        EditFunc edit = nullptr;

        QVector<QString> interfaces;
        bool canBeReplica = false;
    };

    AbstractSimulationObjectModel *createModel(ModeManager *mgr, const QString& objType) const;

    AbstractSimulationObject *createItem(AbstractSimulationObjectModel *model) const;

    SimulationObjectOptionsWidget *createEditWidget(QWidget *parent,
                                                    AbstractSimulationObject *item,
                                                    ViewManager *viewMgr) const;

    // Info
    QStringList getRegisteredTypes() const;
    QStringList typesForInterface(const QString& ifaceName) const;
    QStringList replicaTypes() const;

    QString prettyName(const QString& objType) const;

    // Register
    void registerFactory(const FactoryItem& factory);

private:
    const FactoryItem *getItemForType(const QString& objType) const;

private:
    QVector<FactoryItem> mItems;
};

#endif // SIMULATIONOBJECTFACTORY_H
