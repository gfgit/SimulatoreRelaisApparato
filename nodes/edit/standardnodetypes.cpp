#include "standardnodetypes.h"

#include "../../graph/circuitscene.h"

#include "../../graph/onoffgraphitem.h"
#include "../../nodes/onoffswitchnode.h"

#include "../../graph/powersourcegraphitem.h"
#include "../../nodes/powersourcenode.h"

#include "../../graph/simplenodegraphitem.h"
#include "../../nodes/simplecircuitnode.h"

#include "../../graph/relaispowergraphitem.h"
#include "../../nodes/relaispowernode.h"

#include "../../graph/relaiscontactgraphitem.h"
#include "../../nodes/relaiscontactnode.h"

#include "../../graph/aceibuttongraphitem.h"
#include "../../nodes/aceibuttonnode.h"

#include "../../abstractrelais.h"

#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QCompleter>

#include <QCheckBox>
#include <QSpinBox>

#include "relaylineedit.h"

template <typename Graph>
AbstractNodeGraphItem* addNewNodeToScene(CircuitScene *s)
{
    typename Graph::Node *node = new typename Graph::Node(s);

    if constexpr(std::is_same<typename Graph::Node, RelaisContactNode>())
    {
        node->setRelaisModel(s->relaisModel());
    }
    if constexpr(std::is_same<typename Graph::Node, RelaisPowerNode>())
    {
        node->setRelaisModel(s->relaisModel());
    }

    Graph *graph = new Graph(node);
    return graph;
}

void StandardNodeTypes::registerTypes(NodeEditFactory *factoryReg)
{
    {
        // On/Off
        NodeEditFactory::FactoryItem factory;
        factory.nodeType = OnOffGraphItem::Node::NodeType;
        factory.needsName = true;
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
        factory.edit = [](AbstractNodeGraphItem *item) -> QWidget*
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

            QObject::connect(node, &SimpleCircuitNode::disabledContactChanged,
                             w, updLambda);
            updLambda();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Relais Power
        NodeEditFactory::FactoryItem factory;
        factory.nodeType = RelaisPowerGraphItem::Node::NodeType;
        factory.prettyName = tr("Relay Power");
        factory.create = &addNewNodeToScene<RelaisPowerGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item) -> QWidget*
        {
            RelaisPowerNode *node = static_cast<RelaisPowerNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Relay
            RelayLineEdit *relayEdit = new RelayLineEdit(item->circuitScene()->relaisModel());
            QObject::connect(node, &RelaisPowerNode::relayChanged,
                             relayEdit, &RelayLineEdit::setRelais);
            QObject::connect(relayEdit, &RelayLineEdit::relayChanged,
                             node, &RelaisPowerNode::setRelais);
            relayEdit->setRelais(node->relais());
            lay->addRow(tr("Relay:"), relayEdit);

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Relais Contact
        NodeEditFactory::FactoryItem factory;
        factory.nodeType = RelaisContactGraphItem::Node::NodeType;
        factory.prettyName = tr("Relay Contact");
        factory.create = &addNewNodeToScene<RelaisContactGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item) -> QWidget*
        {
            RelaisContactNode *node = static_cast<RelaisContactNode *>(item->getAbstractNode());

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Relay
            RelayLineEdit *relayEdit = new RelayLineEdit(item->circuitScene()->relaisModel());
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

            auto updLambda = [flipContact, swapContacts, node]()
            {
                flipContact->setChecked(node->flipContact());
                swapContacts->setChecked(node->swapContactState());
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
        factory.needsName = true;
        factory.nodeType = ACEIButtonGraphItem::Node::NodeType;
        factory.prettyName = tr("ACEI Button");
        factory.create = &addNewNodeToScene<ACEIButtonGraphItem>;
        factory.edit = [](AbstractNodeGraphItem *item) -> QWidget*
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
}




