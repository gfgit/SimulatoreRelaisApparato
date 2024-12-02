/**
 * src/objects/standardobjecttypes.cpp
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

#include "standardobjecttypes.h"

#include "simulationobjectfactory.h"

#include "relais/model/abstractrelais.h"
#include "relais/model/relaismodel.h"
#include "relais/view/abstractrelayoptionswidget.h"

#include "lever/acei/aceileverobject.h"
#include "lever/view/genericleveroptionswidget.h"

#include "lever/ace_sasib/acesasiblever5positions.h"
#include "lever/ace_sasib/acesasiblever7positions.h"

#include "simple_activable/lightbulbobject.h"

#include "simple_activable/electromagnet.h"

#include "button/genericbuttonobject.h"

// TODO: extract names in separate header
#include "interfaces/leverinterface.h"
#include "interfaces/mechanicalinterface.h"
#include "interfaces/sasibaceleverextrainterface.h"
#include "interfaces/buttoninterface.h"


#include "interfaces/mechanical/view/genericmechanicaloptionswidget.h"

#include "../views/modemanager.h"

#include "simulationobjectoptionswidget.h"
#include "simulationobjectlineedit.h"
#include <QFormLayout>

#include <QCheckBox>

template <typename T>
AbstractSimulationObjectModel *createModel(ModeManager *mgr)
{
    return new T(mgr);
}

template <typename T>
AbstractSimulationObject *createObject(AbstractSimulationObjectModel *model)
{
    return new T(model);
}

template <typename T, typename W>
QWidget *createEditWidget(AbstractSimulationObject *item)
{
    return new W(static_cast<T*>(item));
}

QWidget *defaultLeverEdit(AbstractSimulationObject *item)
{
    // Generic lever options
    GenericLeverOptionsWidget *genericW
            = new GenericLeverOptionsWidget(item->getInterface<LeverInterface>());
    return genericW;
}

QWidget *defaultMechanicalEdit(AbstractSimulationObject *item)
{
    // Generic mechanical options
    GenericMechanicalOptionsWidget *genericW
            = new GenericMechanicalOptionsWidget(item->model()->modeMgr(),
                                                 item->getInterface<MechanicalInterface>());
    return genericW;
}

QWidget *defaultSasibLeverEdit(AbstractSimulationObject *item)
{
    ACESasibLeverCommonObject *lever = static_cast<ACESasibLeverCommonObject *>(item);

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    // Generic lever options
    lay->addRow(defaultLeverEdit(item));

    // Generic mechanical options
    lay->addRow(defaultMechanicalEdit(item));

    // Electro Magnet
    SimulationObjectLineEdit *magnetEdit
            = new SimulationObjectLineEdit(
                item->model()->modeMgr(),
                {
                    ElectroMagnetObject::Type
                });

    QObject::connect(lever, &ACESasibLeverCommonObject::settingsChanged,
                     magnetEdit, [lever, magnetEdit]()
    {
        magnetEdit->setObject(lever->magnet());
    });
    QObject::connect(magnetEdit, &SimulationObjectLineEdit::objectChanged,
                     lever, [lever](AbstractSimulationObject *obj)
    {
        lever->setMagnet(static_cast<ElectroMagnetObject *>(obj));
    });

    magnetEdit->setObject(lever->magnet());
    lay->addRow(StandardObjectTypes::tr("Magnet"), magnetEdit);

    return w;
}

QWidget *defaultButtonEdit(AbstractSimulationObject *item)
{
    ButtonInterface *buttonIface = item->getInterface<ButtonInterface>();

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    QCheckBox *pressedCB = new QCheckBox(StandardObjectTypes::tr("Can be pressed"));
    lay->addRow(pressedCB);
    pressedCB->setChecked(buttonIface->canBePressed());

    QObject::connect(pressedCB, &QCheckBox::toggled,
                     item, [buttonIface, pressedCB](bool val)
    {
        buttonIface->setCanBePressed(val);
        if(buttonIface->canBePressed() != val)
            pressedCB->setChecked(!val); // Change was rejected
    });

    QCheckBox *extractedCB = new QCheckBox(StandardObjectTypes::tr("Can be extracted"));
    lay->addRow(extractedCB);
    extractedCB->setChecked(buttonIface->canBeExtracted());

    QObject::connect(extractedCB, &QCheckBox::toggled,
                     item, [buttonIface, extractedCB](bool val)
    {
        buttonIface->setCanBeExtracted(val);
        if(buttonIface->canBeExtracted() != val)
            extractedCB->setChecked(!val); // Change was rejected
    });

    return w;
}

void StandardObjectTypes::registerTypes(SimulationObjectFactory *factory)
{
    {
        // Relais
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = &createModel<RelaisModel>;
        item.create = &createObject<AbstractRelais>;
        item.edit = &createEditWidget<AbstractRelais, AbstractRelayOptionsWidget>;
        item.objectType = AbstractRelais::Type;
        item.prettyName = tr("Relais");

        factory->registerFactory(item);
    }

    {
        // ACEI Lever
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<ACEILeverObject>;
        item.edit = &defaultLeverEdit;
        item.objectType = ACEILeverObject::Type;
        item.interfaces = {
            LeverInterface::IfaceType
        };
        item.prettyName = tr("ACEI Lever");

        factory->registerFactory(item);
    }

    {
        // Ligth bulb
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<LightBulbObject>;
        item.edit = nullptr;
        item.objectType = LightBulbObject::Type;
        item.prettyName = tr("Ligth bulb");

        factory->registerFactory(item);
    }

    {
        // Electromagnet
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<ElectroMagnetObject>;
        item.edit = nullptr;
        item.objectType = ElectroMagnetObject::Type;
        item.prettyName = tr("Electromagnet");

        factory->registerFactory(item);
    }

    {
        // Sasib ACE Lever 5 positions
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<ACESasibLever5PosObject>;
        item.objectType = ACESasibLever5PosObject::Type;
        item.interfaces = {
            LeverInterface::IfaceType,
            MechanicalInterface::IfaceType,
            SasibACELeverExtraInterface::IfaceType
        };
        item.prettyName = tr("ACE Sasib 5 Lever");
        item.edit = &defaultSasibLeverEdit;

        factory->registerFactory(item);
    }

    {
        // Sasib ACE Lever 7 positions
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<ACESasibLever7PosObject>;
        item.objectType = ACESasibLever7PosObject::Type;
        item.interfaces = {
            LeverInterface::IfaceType,
            MechanicalInterface::IfaceType,
            SasibACELeverExtraInterface::IfaceType
        };
        item.prettyName = tr("ACE Sasib 7 Lever");
        item.edit = &defaultSasibLeverEdit;

        factory->registerFactory(item);
    }

    {
        // Generic Button
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<GenericButtonObject>;
        item.objectType = GenericButtonObject::Type;
        item.interfaces = {
            ButtonInterface::IfaceType
        };
        item.prettyName = tr("Generic Button");
        item.edit = defaultButtonEdit;

        factory->registerFactory(item);
    }
}
