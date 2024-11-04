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

#include "../graphs/aceibuttongraphitem.h"
#include "../nodes/aceibuttonnode.h"

#include "../graphs/lightbulbgraphitem.h"
#include "../nodes/lightbulbnode.h"


#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QCompleter>

#include <QCheckBox>
#include <QSpinBox>

#include "../../views/modemanager.h"

#include "../../objects/relais/model/abstractrelais.h"
#include "../../objects/relais/view/relaylineedit.h"

template <typename Graph>
AbstractNodeGraphItem* addNewNodeToScene(CircuitScene *s, ModeManager *mgr)
{
    typename Graph::Node *node = new typename Graph::Node(mgr, s);

    Graph *graph = new Graph(node);
    return graph;
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
        factory.edit = nullptr;

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
            RelayLineEdit *relayEdit = new RelayLineEdit(mgr->relaisModel());
            QObject::connect(node, &RelaisPowerNode::relayChanged,
                             relayEdit, &RelayLineEdit::setRelais);
            QObject::connect(relayEdit, &RelayLineEdit::relayChanged,
                             node, &RelaisPowerNode::setRelais);
            relayEdit->setRelais(node->relais());
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

            auto updDelayLambda = [delayUpSeconds, delayDownSeconds, node]()
            {
                delayUpSeconds->setValue(node->delayUpSeconds());
                delayDownSeconds->setValue(node->delayDownSeconds());
            };

            QObject::connect(node, &RelaisPowerNode::delaysChanged,
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
            RelayLineEdit *relayEdit = new RelayLineEdit(mgr->relaisModel());
            QObject::connect(node, &RelaisContactNode::relayChanged,
                             relayEdit, &RelayLineEdit::setRelais);
            QObject::connect(relayEdit, &RelayLineEdit::relayChanged,
                             node, &RelaisContactNode::setRelais);
            relayEdit->setRelais(node->relais());
            lay->addRow(tr("Relay:"), relayEdit);

            QCheckBox *flipContact = new QCheckBox(tr("Flip contact"));
            lay->addRow(flipContact);

            QObject::connect(flipContact, &QCheckBox::toggled,
                             node, [node](bool val)
            {
                node->setFlipContact(val);
            });

            QCheckBox *swapContacts = new QCheckBox(tr("Swap contact state"));
            lay->addRow(swapContacts);

            QObject::connect(swapContacts, &QCheckBox::toggled,
                             node, [node](bool val)
            {
                node->setSwapContactState(val);
            });

            QCheckBox *hasCentralConn = new QCheckBox(tr("Has central connector"));
            lay->addRow(hasCentralConn);

            QObject::connect(hasCentralConn, &QCheckBox::toggled,
                             node, [node](bool val)
            {
                node->setHasCentralConnector(val);
            });

            auto updLambda = [flipContact, swapContacts, hasCentralConn, node]()
            {
                flipContact->setChecked(node->flipContact());
                swapContacts->setChecked(node->swapContactState());
                hasCentralConn->setChecked(node->hasCentralConnector());
            };

            QObject::connect(node, &RelaisContactNode::shapeChanged,
                             w, updLambda);
            updLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // ACEI Button Contact
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Always;
        factory.nodeType = ACEIButtonGraphItem::Node::NodeType;
        factory.prettyName = tr("ACEI Button");
        factory.create = &addNewNodeToScene<ACEIButtonGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item, ModeManager *mgr) -> QWidget*
        {
            ACEIButtonNode *node = static_cast<ACEIButtonNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            QCheckBox *flipContact = new QCheckBox(tr("Flip contact"));
            lay->addRow(flipContact);

            QObject::connect(flipContact, &QCheckBox::toggled,
                             node, [node](bool val)
            {
                node->setFlipContact(val);
            });

            auto updLambda = [flipContact, node]()
            {
                flipContact->setChecked(node->flipContact());
            };

            QObject::connect(node, &ACEIButtonNode::shapeChanged,
                             w, updLambda);
            updLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Light Bulb node
        NodeEditFactory::FactoryItem factory;
        factory.needsName = NodeEditFactory::NeedsName::Always;
        factory.nodeType = LightBulbGraphItem::Node::NodeType;
        factory.prettyName = tr("Light Bulb");
        factory.create = &addNewNodeToScene<LightBulbGraphItem>;
        factory.edit = nullptr;

        factoryReg->registerFactory(factory);
    }
}


