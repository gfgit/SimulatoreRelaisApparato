/**
 * src/objects/simulationobjectfactory.cpp
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

#include "simulationobjectfactory.h"

#include "abstractsimulationobjectmodel.h"
#include "abstractsimulationobject.h"

#include "simulationobjectoptionswidget.h"

SimulationObjectFactory::SimulationObjectFactory()
{

}

AbstractSimulationObjectModel *SimulationObjectFactory::createModel(ModeManager *mgr, const QString &objType) const
{
    const FactoryItem *factory = getItemForType(objType);
    if(!factory)
        return nullptr;

    if(factory->customModelFunc)
        return factory->customModelFunc(mgr);

    // Standard model
    AbstractSimulationObjectModel *m =
            new AbstractSimulationObjectModel(mgr, objType);
    return m;
}

AbstractSimulationObject *SimulationObjectFactory::createItem(AbstractSimulationObjectModel *model) const
{
    const FactoryItem *factory = getItemForType(model->getObjectType());
    if(!factory)
        return nullptr;

    AbstractSimulationObject *item = factory->create(model);
    return item;
}

SimulationObjectOptionsWidget *SimulationObjectFactory::createEditWidget(QWidget *parent,
                                                                         AbstractSimulationObject *item,
                                                                         ViewManager *viewMgr) const
{
    const FactoryItem *factory = getItemForType(item->getType());
    if(!factory)
        return nullptr;

    SimulationObjectOptionsWidget *optionsWidget =
            new SimulationObjectOptionsWidget(item, viewMgr, parent);

    if(factory->edit)
    {
        QWidget *customWidget = factory->edit(item, viewMgr);
        if(customWidget)
            optionsWidget->addCustomWidget(customWidget);
    }

    optionsWidget->fixScrollingChildrenInScrollArea();

    return optionsWidget;
}

QStringList SimulationObjectFactory::getRegisteredTypes() const
{
    QStringList result;
    result.reserve(mItems.size());
    for(const FactoryItem& item : std::as_const(mItems))
        result.append(item.objectType);
    return result;
}

QStringList SimulationObjectFactory::typesForInterface(const QString &ifaceName) const
{
    QStringList result;
    for(const FactoryItem& item : std::as_const(mItems))
    {
        if(item.interfaces.contains(ifaceName))
            result.append(item.objectType);
    }
    return result;
}

QString SimulationObjectFactory::prettyName(const QString &objType) const
{
    const FactoryItem *factory = getItemForType(objType);
    if(!factory)
        return QString();

    return factory->prettyName;
}

void SimulationObjectFactory::registerFactory(const FactoryItem &factory)
{
    mItems.append(factory);
}

const SimulationObjectFactory::FactoryItem *SimulationObjectFactory::getItemForType(const QString &objType) const
{
    for(const FactoryItem& item : std::as_const(mItems))
    {
        if(item.objectType == objType)
            return &item;
    }
    return nullptr;
}
