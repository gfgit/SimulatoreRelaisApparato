/**
 * src/circuits/edit/standardnodetypes.cpp
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

#include "standardnodetypes.h"

#include "../circuitscene.h"

#include "../graphs/onoffgraphitem.h"
#include "../nodes/onoffswitchnode.h"

#include "../graphs/powersourcegraphitem.h"
#include "../nodes/powersourcenode.h"

#include "../graphs/simplenodegraphitem.h"
#include "../nodes/simplecircuitnode.h"

#include "../graphs/relaispowergraphitem.h"
#include "../nodes/relaispowernode.h"

#include "../graphs/relaiscontactgraphitem.h"
#include "../nodes/relaiscontactnode.h"

#include "../graphs/screenrelaiscontactgraphitem.h"
#include "../nodes/screenrelaiscontactnode.h"

#include "../graphs/screenrelaispowergraphitem.h"
#include "../nodes/screenrelaispowernode.h"

#include "../graphs/buttoncontactgraphitem.h"
#include "../nodes/buttoncontactnode.h"

#include "../graphs/lightbulbgraphitem.h"
#include "../nodes/lightbulbnode.h"

#include "../graphs/polarityinversiongraphitem.h"
#include "../nodes/polarityinversionnode.h"

#include "../graphs/electromagnetgraphitem.h"
#include "../nodes/electromagnetnode.h"

#include "../graphs/levercontactgraphitem.h"
#include "../nodes/levercontactnode.h"

#include "../graphs/bifilarizatorgraphitem.h"
#include "../nodes/bifilarizatornode.h"

#include "../graphs/soundcircuitgraphitem.h"
#include "../nodes/soundcircuitnode.h"

#include "../graphs/diodegraphitem.h"
#include "../nodes/diodecircuitnode.h"

// TODO: special
#include "../graphs/special/aceibuttongraphitem.h"
#include "../graphs/special/aceilevergraphitem.h"
#include "../graphs/special/acesasiblevergraphitem.h"

#include <QWidget>
#include <QFormLayout>

#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>

#include "../../views/modemanager.h"

#include "../../objects/simulationobjectlineedit.h"
#include "../../objects/simulationobjectfactory.h"

#include "../../objects/interfaces/buttoninterface.h"
#include "../../objects/interfaces/leverinterface.h"
#include "../../objects/interfaces/sasibaceleverextrainterface.h"

#include "../../objects/simple_activable/abstractsimpleactivableobject.h"
#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../objects/relais/model/abstractrelais.h"
#include "../../objects/screen_relais/model/screenrelais.h"
#include "../../objects/lever/acei/aceileverobject.h"

#include "../../objects/lever/model/levercontactconditionsmodel.h"

#include "../../objects/lever/view/levercontactconditionsview.h"

// TODO: remove BEM
#include "../../objects/lever/bem/bemleverobject.h"

template <typename Graph>
AbstractNodeGraphItem* addNewNodeToScene(CircuitScene *s, ModeManager *mgr)
{
    typename Graph::Node *node = new typename Graph::Node(mgr, s);

    Graph *graph = new Graph(node);
    return graph;
}

QWidget *defaultDeviatorEdit(AbstractDeviatorGraphItem *item, ModeManager *mgr)
{
    AbstractDeviatorNode *node = item->deviatorNode();

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    QCheckBox *flipContact = new QCheckBox(StandardNodeTypes::tr("Flip contact"));
    lay->addRow(flipContact);

    QObject::connect(flipContact, &QCheckBox::toggled,
                     node, [node, flipContact](bool val)
    {
        node->setFlipContact(val);

        // It could have been rejected, check it
        if(node->flipContact() != val)
            flipContact->setChecked(val);
    });

    QCheckBox *swapContacts = new QCheckBox(StandardNodeTypes::tr("Swap contact state"));
    lay->addRow(swapContacts);

    QObject::connect(swapContacts, &QCheckBox::toggled,
                     node, [node, swapContacts](bool val)
    {
        node->setSwapContactState(val);

        // It could have been rejected, check it
        if(node->swapContactState() != val)
            swapContacts->setChecked(val);
    });

    QCheckBox *hasCentralConn = new QCheckBox(StandardNodeTypes::tr("Has central connector"));
    lay->addRow(hasCentralConn);

    QObject::connect(hasCentralConn, &QCheckBox::toggled,
                     node, [node, hasCentralConn](bool val)
    {
        node->setHasCentralConnector(val);

        // It could have been rejected, check it
        if(node->hasCentralConnector() != val)
            hasCentralConn->setChecked(val);
    });

    auto updLambda =
            [flipContact, swapContacts,
            hasCentralConn, node]()
    {
        flipContact->setChecked(node->flipContact());
        swapContacts->setChecked(node->swapContactState());
        hasCentralConn->setChecked(node->hasCentralConnector());

        swapContacts->setVisible(node->allowSwap());
        hasCentralConn->setVisible(node->canChangeCentralConnector());
    };

    QObject::connect(node, &AbstractDeviatorNode::shapeChanged,
                     w, updLambda);
    updLambda();

    return w;
}

QWidget *defaultSimpleActivationEdit(SimpleActivationGraphItem *item, ModeManager *mgr,
                                     const QString& objFieldName)
{
    SimpleActivationNode *node = static_cast<SimpleActivationNode *>(item->getAbstractNode());

    QWidget *w = new QWidget;
    QFormLayout *lay = new QFormLayout(w);

    // Activation Object
    SimulationObjectLineEdit *objectEdit = new SimulationObjectLineEdit(mgr, {node->allowedObjectType()});
    QObject::connect(node, &SimpleActivationNode::objectChanged,
                     objectEdit, &SimulationObjectLineEdit::setObject);
    QObject::connect(objectEdit, &SimulationObjectLineEdit::objectChanged,
                     node, [node](AbstractSimulationObject *obj)
    {
        node->setObject(static_cast<AbstractSimpleActivableObject *>(obj));
    });

    objectEdit->setObject(node->object());
    lay->addRow(objFieldName, objectEdit);

    return w;
}

void StandardNodeTypes::registerTypes(NodeEditFactory *factoryReg)
{
    {
        // On/Off
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Always;
        factory.nodeType = OnOffGraphItem::Node::NodeType;
        factory.prettyName = tr("On/Off switch");
        factory.create = &addNewNodeToScene<OnOffGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            OnOffSwitchNode *node = static_cast<OnOffSwitchNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Initially On
            QCheckBox *initiallyOn = new QCheckBox(tr("Initially On"));
            lay->addRow(initiallyOn);

            QObject::connect(initiallyOn, &QCheckBox::toggled,
                             node, [node](bool val)
            {
                node->setInitiallyOn(val);
            });

            auto updLambda = [initiallyOn, node]()
            {
                initiallyOn->setChecked(node->isInitiallyOn());
            };

            //QObject::connect(node, &OnOffSwitchNode::shapeChanged,
            //                 w, updLambda);
            updLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Power Source
        NodeEditFactory::FactoryItem factory;
        factory.nodeType = PowerSourceGraphItem::Node::NodeType;
        factory.prettyName = tr("Power Source");
        factory.create = &addNewNodeToScene<PowerSourceGraphItem>;
        factory.edit = nullptr;

        factoryReg->registerFactory(factory);
    }

    {
        // Simple Node
        NodeEditFactory::FactoryItem factory;
        factory.nodeType = SimpleNodeGraphItem::Node::NodeType;
        factory.prettyName = tr("Simple Node");
        factory.create = &addNewNodeToScene<SimpleNodeGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            SimpleCircuitNode *node = static_cast<SimpleCircuitNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            QSpinBox *contactSpin = new QSpinBox;
            contactSpin->setRange(0, 3);
            contactSpin->setSpecialValueText(tr("None"));
            lay->addRow(tr("Disabled contact:"), contactSpin);

            QObject::connect(contactSpin, &QSpinBox::valueChanged,
                             node, [node](int val)
            {
                node->setDisabledContact(val);
            });

            auto updLambda = [contactSpin, node]()
            {
                contactSpin->setValue(node->disabledContact());
            };

            QObject::connect(node, &SimpleCircuitNode::shapeChanged,
                             w, updLambda);
            updLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Relais Power
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = RelaisPowerGraphItem::Node::NodeType;
        factory.prettyName = tr("Relay Power");
        factory.create = &addNewNodeToScene<RelaisPowerGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            RelaisPowerNode *node = static_cast<RelaisPowerNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Relay
            SimulationObjectLineEdit *relayEdit = new SimulationObjectLineEdit(mgr, {AbstractRelais::Type});
            QObject::connect(node, &RelaisPowerNode::relayChanged,
                             relayEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(relayEdit, &SimulationObjectLineEdit::objectChanged,
                             node, [node](AbstractSimulationObject *obj)
            {
                node->setRelais(static_cast<AbstractRelais *>(obj));
            });

            relayEdit->setObject(node->relais());
            lay->addRow(tr("Relay:"), relayEdit);

            // Delayed up/down
            QSpinBox *delayUpSeconds = new QSpinBox;
            delayUpSeconds->setRange(0, 5 * 60);
            delayUpSeconds->setSpecialValueText(tr("None"));
            delayUpSeconds->setSuffix(tr(" sec")); // Seconds
            lay->addRow(tr("Delay up:"), delayUpSeconds);

            QObject::connect(delayUpSeconds, &QSpinBox::valueChanged,
                             node, [node](int val)
            {
                node->setDelayUpSeconds(val);
            });

            QSpinBox *delayDownSeconds = new QSpinBox;
            delayDownSeconds->setRange(0, 5 * 60);
            delayDownSeconds->setSpecialValueText(tr("None"));
            delayDownSeconds->setSuffix(tr(" sec")); // Seconds
            lay->addRow(tr("Delay down:"), delayDownSeconds);

            QObject::connect(delayDownSeconds, &QSpinBox::valueChanged,
                             node, [node](int val)
            {
                node->setDelayDownSeconds(val);
            });

            QCheckBox *hasSecondContact = new QCheckBox(tr("Has second contact"));
            lay->addRow(hasSecondContact);

            QObject::connect(hasSecondContact, &QCheckBox::toggled,
                             node, [node, hasSecondContact](bool val)
            {
                node->setHasSecondConnector(val);

                // It could have been rejected, check it
                if(node->hasSecondConnector() != val)
                    hasSecondContact->setChecked(node->hasSecondConnector());
            });

            auto updDelayLambda =
                    [delayUpSeconds, delayDownSeconds,
                    hasSecondContact,
                    node]()
            {
                delayUpSeconds->setValue(node->delayUpSeconds());
                delayDownSeconds->setValue(node->delayDownSeconds());
                hasSecondContact->setChecked(node->hasSecondConnector());
            };

            QObject::connect(node, &RelaisPowerNode::delaysChanged,
                             w, updDelayLambda);
            QObject::connect(node, &RelaisPowerNode::shapeChanged,
                             w, updDelayLambda);
            updDelayLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Relais Contact
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = RelaisContactGraphItem::Node::NodeType;
        factory.prettyName = tr("Relay Contact");
        factory.create = &addNewNodeToScene<RelaisContactGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            RelaisContactNode *node = static_cast<RelaisContactNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Relay
            SimulationObjectLineEdit *relayEdit = new SimulationObjectLineEdit(mgr, {AbstractRelais::Type});
            QObject::connect(node, &RelaisContactNode::relayChanged,
                             relayEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(relayEdit, &SimulationObjectLineEdit::objectChanged,
                             node, [node](AbstractSimulationObject *obj)
            {
                node->setRelais(static_cast<AbstractRelais *>(obj));
            });

            relayEdit->setObject(node->relais());
            lay->addRow(tr("Relay:"), relayEdit);

            // Deviator
            lay->addWidget(defaultDeviatorEdit(static_cast<AbstractDeviatorGraphItem *>(item),
                                               mgr));

            QCheckBox *hideRelayNormal = new QCheckBox(tr("Hide relay normal state"));
            lay->addWidget(hideRelayNormal);

            QObject::connect(hideRelayNormal, &QCheckBox::toggled,
                             node, [node](bool val)
            {
                node->setHideRelayNormalState(val);
            });

            auto updLambda =
                    [hideRelayNormal, node]()
            {
                hideRelayNormal->setChecked(node->hideRelayNormalState());
            };

            QObject::connect(node, &RelaisContactNode::shapeChanged,
                             w, updLambda);
            updLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Screen Relais Power
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = ScreenRelaisPowerGraphItem::Node::NodeType;
        factory.prettyName = tr("Screen Relay Power");
        factory.create = &addNewNodeToScene<ScreenRelaisPowerGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            ScreenRelaisPowerNode *node = static_cast<ScreenRelaisPowerNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Screen Relay
            SimulationObjectLineEdit *relayEdit = new SimulationObjectLineEdit(mgr, {ScreenRelais::Type});
            QObject::connect(node, &ScreenRelaisPowerNode::relayChanged,
                             relayEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(relayEdit, &SimulationObjectLineEdit::objectChanged,
                             node, [node](AbstractSimulationObject *obj)
            {
                node->setScreenRelais(static_cast<ScreenRelais *>(obj));
            });

            relayEdit->setObject(node->screenRelais());
            lay->addRow(tr("Screen Relay:"), relayEdit);

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Screen Relais Contact
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = ScreenRelaisContactGraphItem::Node::NodeType;
        factory.prettyName = tr("Screen Relay Contact");
        factory.create = &addNewNodeToScene<ScreenRelaisContactGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            ScreenRelaisContactNode *node = static_cast<ScreenRelaisContactNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Relay
            SimulationObjectLineEdit *relayEdit = new SimulationObjectLineEdit(mgr, {ScreenRelais::Type});
            QObject::connect(node, &ScreenRelaisContactNode::relayChanged,
                             relayEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(relayEdit, &SimulationObjectLineEdit::objectChanged,
                             node, [node](AbstractSimulationObject *obj)
            {
                node->setScreenRelais(static_cast<ScreenRelais *>(obj));
            });

            relayEdit->setObject(node->screenRelais());
            lay->addRow(tr("Screen Relay:"), relayEdit);

            // Deviator
            lay->addWidget(defaultDeviatorEdit(static_cast<AbstractDeviatorGraphItem *>(item),
                                               mgr));

            QCheckBox *contactA = new QCheckBox(tr("Contact A"));
            lay->addWidget(contactA);

            QObject::connect(contactA, &QCheckBox::toggled,
                             node, [node](bool val)
            {
                node->setIsContactA(val);
            });

            auto updLambda =
                    [contactA, node]()
            {
                contactA->setChecked(node->isContactA());
            };

            QObject::connect(node, &ScreenRelaisContactNode::shapeChanged,
                             w, updLambda);
            updLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Button Contact
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = ButtonContactGraphItem::Node::NodeType;
        factory.prettyName = tr("Button Contact");
        factory.create = &addNewNodeToScene<ButtonContactGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            ButtonContactNode *node = static_cast<ButtonContactNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Deviator
            lay->addWidget(defaultDeviatorEdit(static_cast<AbstractDeviatorGraphItem *>(item),
                                               mgr));

            // Button
            const QStringList buttonTypes = mgr->objectFactory()
                    ->typesForInterface(ButtonInterface::IfaceType);

            SimulationObjectLineEdit *buttonEdit =
                    new SimulationObjectLineEdit(mgr, buttonTypes);

            QObject::connect(node, &ButtonContactNode::buttonChanged,
                             buttonEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(buttonEdit, &SimulationObjectLineEdit::objectChanged,
                             node, [node](AbstractSimulationObject *obj)
            {
                node->setButton(obj);
            });
            buttonEdit->setObject(node->button());

            lay->addRow(tr("Button:"), buttonEdit);

            // Button contacts state
            QGridLayout *gridLay = new QGridLayout;
            QCheckBox *firstArr[3] = {nullptr};
            QCheckBox *secondArr[3] = {nullptr};

            const QString stateNames[3] =
            {
                tr("Normal"),
                tr("Pressed"),
                tr("Extracted")
            };

            for(int i = 0; i < 3; i++)
            {
                gridLay->addWidget(new QLabel(tr("When %1:").arg(stateNames[i])), i, 0);

                firstArr[i] = new QCheckBox("First");
                gridLay->addWidget(firstArr[i], i, 1);

                secondArr[i] = new QCheckBox("Second");
                gridLay->addWidget(secondArr[i], i, 2);
            }

            lay->setLayout(lay->rowCount(), QFormLayout::SpanningRole, gridLay);

            auto setNewState = [node, firstArr, secondArr]()
            {
                for(int i = 0; i < 3; i++)
                {
                    node->setContactStateFor(i, 0, firstArr[i]->isChecked());
                    node->setContactStateFor(i, 1, secondArr[i]->isChecked());
                }
            };

            auto updateState = [node, firstArr, secondArr]()
            {
                for(int i = 0; i < 3; i++)
                {
                    firstArr[i]->setChecked(node->getContactStateFor(i, 0));
                    secondArr[i]->setChecked(node->getContactStateFor(i, 1));
                }
            };

            updateState();

            for(int i = 0; i < 3; i++)
            {
                QObject::connect(firstArr[i], &QCheckBox::toggled,
                                 node, setNewState);
                QObject::connect(secondArr[i], &QCheckBox::toggled,
                                 node, setNewState);
            }

            QObject::connect(node, &ButtonContactNode::contactStateSettingsChanged,
                             w, updateState);

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Light Bulb node
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = LightBulbGraphItem::Node::NodeType;
        factory.prettyName = tr("Light Bulb");
        factory.create = &addNewNodeToScene<LightBulbGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            return defaultSimpleActivationEdit(static_cast<SimpleActivationGraphItem *>(item),
                                               mgr, tr("Light:"));
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Electromagnet node
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = ElectroMagnetGraphItem::Node::NodeType;
        factory.prettyName = tr("Electromagnet");
        factory.create = &addNewNodeToScene<ElectroMagnetGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            return defaultSimpleActivationEdit(static_cast<SimpleActivationGraphItem *>(item),
                                               mgr, tr("Magnet:"));
        };

        factoryReg->registerFactory(factory);
    }

    {
        // ACEI Button
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = ACEIButtonGraphItem::CustomNodeType;
        factory.prettyName = tr("ACEI Button");
        factory.create = &addNewNodeToScene<ACEIButtonGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            ACEIButtonGraphItem *specialItem = static_cast<ACEIButtonGraphItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Button
            const QStringList buttonTypes = mgr->objectFactory()
                    ->typesForInterface(ButtonInterface::IfaceType);

            SimulationObjectLineEdit *buttonEdit =
                    new SimulationObjectLineEdit(mgr, buttonTypes);

            QObject::connect(specialItem, &ACEIButtonGraphItem::buttonChanged,
                             buttonEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(buttonEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setButton(obj);
            });
            buttonEdit->setObject(specialItem->button());

            lay->addRow(tr("Button:"), buttonEdit);

            // Central Light
            SimulationObjectLineEdit *centralLightEdit =
                    new SimulationObjectLineEdit(mgr, {LightBulbObject::Type});
            QObject::connect(centralLightEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setCentralLight(static_cast<LightBulbObject *>(obj));
            });

            lay->addRow(tr("Central light:"), centralLightEdit);

            auto updateLights = [specialItem, centralLightEdit]()
            {
                centralLightEdit->setObject(specialItem->centralLight());
            };

            QObject::connect(specialItem, &ACEIButtonGraphItem::lightsChanged,
                             w, updateLights);

            updateLights();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // ACEI Lever
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = ACEILeverGraphItem::CustomNodeType;
        factory.prettyName = tr("ACEI Lever");
        factory.create = &addNewNodeToScene<ACEILeverGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            ACEILeverGraphItem *specialItem = static_cast<ACEILeverGraphItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Lever
            // TODO: remove BEM
            SimulationObjectLineEdit *leverEdit = new SimulationObjectLineEdit(mgr, {ACEILeverObject::Type, BEMLeverObject::Type});
            QObject::connect(specialItem, &ACEILeverGraphItem::leverChanged,
                             leverEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(leverEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setLever(obj);
            });
            leverEdit->setObject(specialItem->lever());

            lay->addRow(tr("Lever:"), leverEdit);

            // Left Light
            SimulationObjectLineEdit *leftLightEdit =
                    new SimulationObjectLineEdit(mgr, {LightBulbObject::Type});
            QObject::connect(leftLightEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setLeftLight(static_cast<LightBulbObject *>(obj));
            });

            lay->addRow(tr("Left light:"), leftLightEdit);

            // Right Light
            SimulationObjectLineEdit *rightLightEdit =
                    new SimulationObjectLineEdit(mgr, {LightBulbObject::Type});
            QObject::connect(rightLightEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setRightLight(static_cast<LightBulbObject *>(obj));
            });

            lay->addRow(tr("Right light:"), rightLightEdit);

            auto updateLights = [specialItem, leftLightEdit, rightLightEdit]()
            {
                leftLightEdit->setObject(specialItem->leftLight());
                rightLightEdit->setObject(specialItem->rightLight());
            };

            QObject::connect(specialItem, &ACEILeverGraphItem::lightsChanged,
                             w, updateLights);

            updateLights();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // ACE Sasib Lever
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = ACESasibLeverGraphItem::CustomNodeType;
        factory.prettyName = tr("ACE Sasib Lever");
        factory.create = &addNewNodeToScene<ACESasibLeverGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            ACESasibLeverGraphItem *specialItem = static_cast<ACESasibLeverGraphItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Lever
            const QStringList sasibTypes = mgr->objectFactory()
                    ->typesForInterface(SasibACELeverExtraInterface::IfaceType);

            SimulationObjectLineEdit *leverEdit =
                    new SimulationObjectLineEdit(
                        mgr,
                        sasibTypes);

            QObject::connect(specialItem, &ACESasibLeverGraphItem::leverChanged,
                             leverEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(leverEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setLever(obj);
            });
            leverEdit->setObject(specialItem->lever());

            lay->addRow(tr("Lever:"), leverEdit);

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Lever Contact
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = LeverContactGraphItem::Node::NodeType;
        factory.prettyName = tr("Lever Contact");
        factory.create = &addNewNodeToScene<LeverContactGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            LeverContactNode *node = static_cast<LeverContactNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Lever
            const QStringList leverTypes = mgr->objectFactory()
                    ->typesForInterface(LeverInterface::IfaceType);

            SimulationObjectLineEdit *leverEdit =
                    new SimulationObjectLineEdit(
                        mgr,
                        leverTypes);

            QObject::connect(leverEdit, &SimulationObjectLineEdit::objectChanged,
                             node, [node](AbstractSimulationObject *obj)
            {
                node->setLever(obj);
            });
            leverEdit->setObject(node->lever());
            lay->addRow(tr("Lever:"), leverEdit);

            // Deviator
            lay->addWidget(defaultDeviatorEdit(static_cast<AbstractDeviatorGraphItem *>(item),
                                               mgr));

            // Conditions
            LeverContactConditionsModel *conditionsModel =
                    new LeverContactConditionsModel(w);

            LeverContactConditionsView *conditionsView =
                    new LeverContactConditionsView;
            lay->addWidget(new QLabel(tr("Conditions")));
            lay->addWidget(conditionsView);

            conditionsView->setModel(conditionsModel);

            QObject::connect(node, &LeverContactNode::leverChanged,
                             w,
                             [leverEdit, conditionsModel,
                             node, conditionsView]()
            {
                leverEdit->setObject(node->lever());

                if(node->lever())
                {
                    LeverInterface *leverIface = node->lever()->getInterface<LeverInterface>();

                    auto conditions = conditionsModel->conditions();
                    conditionsModel->setConditions(leverIface->positionDesc(),
                                                   conditions,
                                                   leverIface->canWarpAroundZero());
                    conditionsModel->setPositionRange(leverIface->absoluteMin(),
                                                      leverIface->absoluteMax());
                }

                conditionsView->setVisible(node->lever());
            });

            if(node->lever())
            {
                LeverInterface *leverIface = node->lever()->getInterface<LeverInterface>();

                auto conditions = node->conditionSet();
                conditionsModel->setConditions(leverIface->positionDesc(),
                                               conditions,
                                               leverIface->canWarpAroundZero());
                conditionsModel->setPositionRange(leverIface->absoluteMin(),
                                                  leverIface->absoluteMax());
            }

            conditionsView->setVisible(node->lever());

            QObject::connect(conditionsModel, &LeverContactConditionsModel::changed,
                             node, [node, conditionsModel]()
            {
                // Apply changes
                node->setConditionSet(conditionsModel->conditions());
            });

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Polarity inversion node
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::OnlyOnEditing;
        factory.nodeType = PolarityInversionGraphItem::Node::NodeType;
        factory.prettyName = tr("Polarity Inversion");
        factory.create = &addNewNodeToScene<PolarityInversionGraphItem>;
        factory.edit = nullptr;

        factoryReg->registerFactory(factory);
    }

    {
        // Bifilarizator node
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::OnlyOnEditing;
        factory.nodeType = BifilarizatorGraphItem::Node::NodeType;
        factory.prettyName = tr("BiFiLar1zaT0R");
        factory.create = &addNewNodeToScene<BifilarizatorGraphItem>;
        factory.edit = nullptr;

        factoryReg->registerFactory(factory);
    }

    {
        // Sound Circuit node
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Never;
        factory.nodeType = SoundCircuitGraphItem::Node::NodeType;
        factory.prettyName = tr("Sound Node");
        factory.create = &addNewNodeToScene<SoundCircuitGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            return defaultSimpleActivationEdit(static_cast<SimpleActivationGraphItem *>(item), mgr,
                                               tr("Sound Object"));
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Diode Circuit node
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::OnlyOnEditing;
        factory.nodeType = DiodeGraphItem::Node::NodeType;
        factory.prettyName = tr("Diode");
        factory.create = &addNewNodeToScene<DiodeGraphItem>;
        factory.edit = nullptr;

        factoryReg->registerFactory(factory);
    }
}


