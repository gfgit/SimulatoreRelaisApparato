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

#include "screen_relais/model/screenrelais.h"
#include "screen_relais/model/screenrelaismodel.h"
#include "screen_relais/view/screenrelaisoptionswidget.h"

#include "lever/acei/aceileverobject.h"
#include "lever/view/genericleveroptionswidget.h"

#include "lever/ace_sasib/acesasiblever5positions.h"
#include "lever/ace_sasib/acesasiblever7positions.h"

#include "lever/bem/bemleverobject.h"

#include "simple_activable/lightbulbobject.h"

#include "simple_activable/electromagnet.h"

#include "simple_activable/soundobject.h"

#include "button/genericbuttonobject.h"

#include "circuit_bridge/remotecircuitbridge.h"
#include "circuit_bridge/remotecircuitbridgesmodel.h"

// TODO: extract names in separate header
#include "interfaces/leverinterface.h"
#include "interfaces/mechanicalinterface.h"
#include "interfaces/sasibaceleverextrainterface.h"
#include "interfaces/buttoninterface.h"
#include "interfaces/bemhandleinterface.h"


#include "interfaces/mechanical/view/genericmechanicaloptionswidget.h"

#include "../views/modemanager.h"

#include "simulationobjectoptionswidget.h"
#include "simulationobjectlineedit.h"
#include <QFormLayout>

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include <QFileDialog>

#include <QComboBox>
#include "../utils/enumvaluesmodel.h"

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
QWidget *createEditWidget(AbstractSimulationObject *item, ViewManager *mgr)
{
    Q_UNUSED(mgr)
    return new W(static_cast<T*>(item));
}

QWidget *defaultLeverEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    // Generic lever options
    GenericLeverOptionsWidget *genericW
            = new GenericLeverOptionsWidget(item->getInterface<LeverInterface>());
    return genericW;
}

QWidget *defaultMechanicalEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    // Generic mechanical options
    GenericMechanicalOptionsWidget *genericW
            = new GenericMechanicalOptionsWidget(mgr,
                                                 item->getInterface<MechanicalInterface>());
    return genericW;
}

QWidget *defaultSasibLeverEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    ACESasibLeverCommonObject *lever = static_cast<ACESasibLeverCommonObject *>(item);

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    // Generic lever options
    lay->addRow(defaultLeverEdit(item, mgr));

    // Generic mechanical options
    lay->addRow(defaultMechanicalEdit(item, mgr));

    // Electro Magnet
    SimulationObjectLineEdit *magnetEdit
            = new SimulationObjectLineEdit(
                mgr,
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

QWidget *defaultACEILeverEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    ACEILeverObject *lever = static_cast<ACEILeverObject *>(item);

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    // Generic lever options
    lay->addRow(defaultLeverEdit(item, mgr));

    // Security seal
    QCheckBox *leftSealCB = new QCheckBox(StandardObjectTypes::tr("Left position can be security-sealed"));
    lay->addRow(leftSealCB);

    QObject::connect(leftSealCB, &QCheckBox::toggled,
                     item, [lever](bool val)
    {
        lever->setCanSealLeftPosition(val);
    });

    auto updateSettings = [lever, leftSealCB]()
    {
        leftSealCB->setChecked(lever->canSealLeftPosition());
        leftSealCB->setEnabled(lever->getInterface<LeverInterface>()->absoluteMin() == int(ACEILeverPosition::Left));
    };

    QObject::connect(item, &AbstractSimulationObject::settingsChanged,
                     leftSealCB, updateSettings);

    updateSettings();

    return w;
}

QWidget *defaultButtonEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    ButtonInterface *buttonIface = item->getInterface<ButtonInterface>();

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    QCheckBox *pressedCB = new QCheckBox(StandardObjectTypes::tr("Can be pressed"));
    lay->addRow(pressedCB);

    QObject::connect(pressedCB, &QCheckBox::toggled,
                     item, [buttonIface, pressedCB](bool val)
    {
        buttonIface->setCanBePressed(val);
        if(buttonIface->canBePressed() != val)
            pressedCB->setChecked(!val); // Change was rejected
    });

    QCheckBox *extractedCB = new QCheckBox(StandardObjectTypes::tr("Can be extracted"));
    lay->addRow(extractedCB);

    QObject::connect(extractedCB, &QCheckBox::toggled,
                     item, [buttonIface, extractedCB](bool val)
    {
        buttonIface->setCanBeExtracted(val);
        if(buttonIface->canBeExtracted() != val)
            extractedCB->setChecked(!val); // Change was rejected
    });

    // Mode
    QComboBox *modeCombo = new QComboBox;
    EnumValuesModel *modeModel = new EnumValuesModel(modeCombo);
    modeModel->setEnumDescFull(ButtonInterface::getModeDesc(), false);
    modeCombo->setModel(modeModel);

    QObject::connect(modeCombo, &QComboBox::activated,
                     item, [buttonIface, modeModel](int idx)
    {
        buttonIface->setMode(ButtonInterface::Mode(modeModel->valueAt(idx)));
    });

    lay->addRow(StandardObjectTypes::tr("Mode:"), modeCombo);

    auto updateSettings = [buttonIface, pressedCB, extractedCB, modeCombo, modeModel]()
    {
        pressedCB->setChecked(buttonIface->canBePressed());
        extractedCB->setChecked(buttonIface->canBeExtracted());
        modeCombo->setCurrentIndex(modeModel->rowForValue(int(buttonIface->mode())));
    };

    QObject::connect(item, &AbstractSimulationObject::settingsChanged,
                     modeCombo, updateSettings);

    updateSettings();

    return w;
}

QWidget *soundObjectEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    SoundObject *sound = static_cast<SoundObject *>(item);

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    // Loop
    QCheckBox *loopCheckBox = new QCheckBox(StandardObjectTypes::tr("Loop sound"));
    lay->addRow(loopCheckBox);

    loopCheckBox->setChecked(sound->loopEnabled());
    QObject::connect(loopCheckBox, &QCheckBox::toggled,
                     sound, [sound](bool val)
    {
        sound->setLoopEnabled(val);
    });

    // Sound File
    QLineEdit *soundFileEdit = new QLineEdit;
    lay->addRow(StandardObjectTypes::tr("Sound File:"), soundFileEdit);

    QPushButton *browseBut = new QPushButton(StandardObjectTypes::tr("Browse"));
    lay->addRow(browseBut);

    QPushButton *applyBut = new QPushButton(StandardObjectTypes::tr("Apply"));
    lay->addRow(applyBut);

    soundFileEdit->setText(sound->getSoundFile());

    QObject::connect(applyBut, &QPushButton::clicked,
                     sound, [sound, soundFileEdit]()
    {
        sound->setSoundFile(soundFileEdit->text());
    });

    QObject::connect(browseBut, &QPushButton::clicked,
                     soundFileEdit, [soundFileEdit]()
    {
        QString str = QFileDialog::getOpenFileName(soundFileEdit,
                                                   StandardObjectTypes::tr("Choose WAV Sound"),
                                                   soundFileEdit->text());
        soundFileEdit->setText(str);
    });

    return w;
}

QWidget *defaultBEMLeverEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    BEMLeverObject *lever = static_cast<BEMLeverObject *>(item);
    BEMHandleInterface *bemIface = lever->getInterface<BEMHandleInterface>();

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    // Generic lever options
    lay->addRow(defaultLeverEdit(item, mgr));

    // BEM Interface

    // Type
    QComboBox *typeCombo = new QComboBox;
    EnumValuesModel *typeModel = new EnumValuesModel(typeCombo);
    typeModel->setEnumDescFull(BEMHandleInterface::getLeverTypeDesc(), false);
    typeCombo->setModel(typeModel);

    QObject::connect(typeCombo, &QComboBox::activated,
                     lever, [bemIface, typeModel](int idx)
    {
        bemIface->setLeverType(BEMHandleInterface::LeverType(typeModel->valueAt(idx)));
    });

    lay->addRow(StandardObjectTypes::tr("Type:"), typeCombo);

    // Twin Handle
    SimulationObjectLineEdit *twinEdit
            = new SimulationObjectLineEdit(
                mgr,
                {
                    BEMLeverObject::Type
                });

    QObject::connect(twinEdit, &SimulationObjectLineEdit::objectChanged,
                     lever, [bemIface, twinEdit](AbstractSimulationObject *obj)
    {
        bemIface->setTwinHandle(obj ?
                                    obj->getInterface<BEMHandleInterface>() :
                                    nullptr);

        AbstractSimulationObject *actualObj = bemIface->getTwinHandle() ?
                    bemIface->getTwinHandle()->object() : nullptr;
        if(actualObj != obj)
            twinEdit->setObject(actualObj);
    });

    lay->addRow(StandardObjectTypes::tr("Twin Handle:"), twinEdit);

    // Liberation Relay
    SimulationObjectLineEdit *relayEdit
            = new SimulationObjectLineEdit(
                mgr,
                {
                    AbstractRelais::Type
                });

    QObject::connect(relayEdit, &SimulationObjectLineEdit::objectChanged,
                     lever, [bemIface](AbstractSimulationObject *obj)
    {
        bemIface->setLiberationRelay(qobject_cast<AbstractRelais *>(obj));
    });

    lay->addRow(StandardObjectTypes::tr("Liberation Relay:"), relayEdit);

    // Artificial Liberation Button
    SimulationObjectLineEdit *butEdit
            = new SimulationObjectLineEdit(
                mgr,
                item->model()->modeMgr()->objectFactory()
                ->typesForInterface(ButtonInterface::IfaceType));

    QObject::connect(butEdit, &SimulationObjectLineEdit::objectChanged,
                     lever, [bemIface](AbstractSimulationObject *obj)
    {
        bemIface->setArtificialLiberation(obj->getInterface<ButtonInterface>());
    });

    lay->addRow(StandardObjectTypes::tr("Artificial Liberation Button:"), butEdit);

    auto updateSettings = [bemIface, twinEdit, typeCombo, typeModel, relayEdit, butEdit, lay]()
    {
        twinEdit->setObject(bemIface->getTwinHandle() ?
                                bemIface->getTwinHandle()->object() :
                                nullptr);
        typeCombo->setCurrentIndex(typeModel->rowForValue(int(bemIface->leverType())));

        relayEdit->setObject(bemIface->liberationRelay());

        butEdit->setObject(bemIface->artificialLiberation() ?
                               bemIface->artificialLiberation()->object() :
                               nullptr);

        // Show Liberation relay only for Consensus levers
        const bool consensus = bemIface->leverType() == BEMHandleInterface::LeverType::Consensus;
        lay->setRowVisible(relayEdit, consensus);
        lay->setRowVisible(butEdit, consensus);
    };

    QObject::connect(lever, &BEMLeverObject::settingsChanged,
                     twinEdit, updateSettings);

    updateSettings();

    return w;
}

QWidget *defaultCircuitBridgeEdit(AbstractSimulationObject *item, ViewManager *mgr)
{
    RemoteCircuitBridge *bridge = static_cast<RemoteCircuitBridge *>(item);

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    // Node Description
    QLineEdit *nodeDescrA = new QLineEdit;
    QLineEdit *nodeDescrB = new QLineEdit;

    QObject::connect(nodeDescrA, &QLineEdit::textEdited,
                     bridge, [bridge, nodeDescrA]()
    {
        bridge->setNodeDescription(true, nodeDescrA->text());
    });

    QObject::connect(nodeDescrB, &QLineEdit::textEdited,
                     bridge, [bridge, nodeDescrB]()
    {
        bridge->setNodeDescription(false, nodeDescrB->text());
    });

    lay->addRow(StandardObjectTypes::tr("Description A:"), nodeDescrA);
    lay->addRow(StandardObjectTypes::tr("Description B:"), nodeDescrB);

    nodeDescrA->setPlaceholderText(StandardObjectTypes::tr("Shown on node A"));
    nodeDescrB->setPlaceholderText(StandardObjectTypes::tr("Shown on node B"));

    QPalette normalPalette = nodeDescrA->palette();
    QPalette redTextPalette = normalPalette;
    redTextPalette.setColor(QPalette::Text, Qt::red);

    // Remote session
    QCheckBox *remoteCB = new QCheckBox(StandardObjectTypes::tr("To Remote Node"));
    lay->addRow(remoteCB);

    QObject::connect(remoteCB, &QCheckBox::toggled,
                     bridge, [bridge](bool val)
    {
        bridge->setRemote(val);
    });

    QLineEdit *sessionEdit = new QLineEdit;
    lay->addRow(StandardObjectTypes::tr("Peer Session:"), sessionEdit);
    QObject::connect(sessionEdit, &QLineEdit::textEdited,
                     bridge, [bridge, sessionEdit]()
    {
        bridge->setRemoteSessionName(sessionEdit->text());
    });

    QLineEdit *peerNodeEdit = new QLineEdit;
    lay->addRow(StandardObjectTypes::tr("Peer Node:"), peerNodeEdit);
    QObject::connect(peerNodeEdit, &QLineEdit::textEdited,
                     bridge, [bridge, peerNodeEdit]()
    {
        bridge->setPeerNodeName(peerNodeEdit->text());
    });

    QCheckBox *serialCB = new QCheckBox(StandardObjectTypes::tr("Use serial device"));
    lay->addRow(serialCB);

    QObject::connect(serialCB, &QCheckBox::toggled,
                     bridge, [bridge](bool val)
    {
        bridge->setUseSerial(val);
    });

    QLineEdit *serialDeviceEdit = new QLineEdit;
    lay->addRow(StandardObjectTypes::tr("Device Name:"), serialDeviceEdit);
    QObject::connect(serialDeviceEdit, &QLineEdit::textEdited,
                     bridge, [bridge, serialDeviceEdit]()
    {
        bridge->setDeviceName(serialDeviceEdit->text());
    });

    QSpinBox *inputIdSpin = new QSpinBox;
    inputIdSpin->setRange(0, 255);
    inputIdSpin->setSpecialValueText(StandardObjectTypes::tr("None"));
    lay->addRow(StandardObjectTypes::tr("Input ID:"), inputIdSpin);
    QObject::connect(inputIdSpin, &QSpinBox::editingFinished,
                     bridge, [bridge, inputIdSpin]()
    {
        bridge->setSerialInputId(inputIdSpin->value());
        if(inputIdSpin->value() != bridge->serialInputId())
            inputIdSpin->setValue(bridge->serialInputId()); // Rejected
    });

    QSpinBox *outputIdSpin = new QSpinBox;
    outputIdSpin->setRange(0, 255);
    outputIdSpin->setSpecialValueText(StandardObjectTypes::tr("None"));
    lay->addRow(StandardObjectTypes::tr("Output ID:"), outputIdSpin);
    QObject::connect(outputIdSpin, &QSpinBox::editingFinished,
                     bridge, [bridge, outputIdSpin]()
    {
        bridge->setSerialOutputId(outputIdSpin->value());
        if(outputIdSpin->value() != bridge->serialOutputId())
            outputIdSpin->setValue(bridge->serialOutputId()); // Rejected
    });

    auto updateSettings = [bridge, normalPalette, redTextPalette,
            nodeDescrA, nodeDescrB,
            remoteCB, sessionEdit, peerNodeEdit,
            serialCB, serialDeviceEdit, inputIdSpin, outputIdSpin]()
    {
        const QString descrA = bridge->getNodeDescription(true);
        if(nodeDescrA->text() != descrA)
            nodeDescrA->setText(descrA);

        const QString descrB = bridge->getNodeDescription(false);
        if(nodeDescrB->text() != descrB)
            nodeDescrB->setText(descrB);

        const bool hasNodeA = bridge->getNode(true);
        nodeDescrA->setPalette(hasNodeA ? normalPalette : redTextPalette);
        nodeDescrA->setToolTip(hasNodeA ? QString()
                                        : StandardObjectTypes::tr("Node A not set!"));

        const bool hasNodeB = bridge->getNode(false);
        nodeDescrB->setPalette(hasNodeB ? normalPalette : redTextPalette);
        nodeDescrB->setToolTip(hasNodeB ? QString()
                                        : StandardObjectTypes::tr("Node B not set!"));

        remoteCB->setChecked(bridge->isRemote());
        remoteCB->setEnabled(!hasNodeA || !hasNodeB);

        sessionEdit->setEnabled(remoteCB->isChecked());
        peerNodeEdit->setEnabled(remoteCB->isChecked());

        serialCB->setChecked(bridge->getUseSerial());
        serialCB->setEnabled(!hasNodeA || !hasNodeB);

        serialDeviceEdit->setText(bridge->getDeviceName());
        inputIdSpin->setValue(bridge->serialInputId());
        outputIdSpin->setValue(bridge->serialOutputId());

        QString str = bridge->remoteSessionName();
        if(str != sessionEdit->text())
            sessionEdit->setText(str);

        str = bridge->peerNodeName();
        if(str != peerNodeEdit->text())
            peerNodeEdit->setText(str);
    };

    QObject::connect(bridge, &RemoteCircuitBridge::settingsChanged,
                     w, updateSettings);

    updateSettings();

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
        // Screen Relais
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = &createModel<ScreenRelaisModel>;
        item.create = &createObject<ScreenRelais>;
        item.edit = &createEditWidget<ScreenRelais, ScreenRelaisOptionsWidget>;
        item.objectType = ScreenRelais::Type;
        item.prettyName = tr("Screen Relais");

        factory->registerFactory(item);
    }

    {
        // ACEI Lever
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<ACEILeverObject>;
        item.edit = &defaultACEILeverEdit;
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

    {
        // Sound Activable Object
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<SoundObject>;
        item.objectType = SoundObject::Type;
        item.prettyName = tr("Sound Object");
        item.edit = &soundObjectEdit;

        factory->registerFactory(item);
    }

    {
        // BEM Lever
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = nullptr;
        item.create = &createObject<BEMLeverObject>;
        item.edit = &defaultBEMLeverEdit;
        item.objectType = BEMLeverObject::Type;
        item.interfaces = {
            LeverInterface::IfaceType
        };
        item.prettyName = tr("BEM Handle");

        factory->registerFactory(item);
    }

    {
        // Remote Circuit Bridge
        SimulationObjectFactory::FactoryItem item;
        item.customModelFunc = &createModel<RemoteCircuitBridgesModel>;
        item.create = &createObject<RemoteCircuitBridge>;
        item.edit = &defaultCircuitBridgeEdit;
        item.objectType = RemoteCircuitBridge::Type;
        item.prettyName = tr("Circuit Bridge");

        factory->registerFactory(item);
    }
}
