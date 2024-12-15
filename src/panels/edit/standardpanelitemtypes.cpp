/**
 * src/panels/edit/standardpanelitemtypes.cpp
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

#include "standardpanelitemtypes.h"
#include "panelitemfactory.h"

#include "../panelscene.h"

#include "../abstractpanelitem.h"

// TODO: special
//#include "../graphs/special/aceibuttongraphitem.h"
//#include "../graphs/special/aceilevergraphitem.h"
//#include "../graphs/special/acesasiblevergraphitem.h"

#include "../graphs/lightrectitem.h"

#include <QWidget>
#include <QFormLayout>

#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

#include "../../views/modemanager.h"

#include "../../objects/simulationobjectlineedit.h"
#include "../../objects/simulationobjectfactory.h"

#include "../../objects/interfaces/buttoninterface.h"
#include "../../objects/interfaces/leverinterface.h"
#include "../../objects/interfaces/sasibaceleverextrainterface.h"

#include "../../objects/simple_activable/abstractsimpleactivableobject.h"
#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../objects/relais/model/abstractrelais.h"
#include "../../objects/lever/acei/aceileverobject.h"

#include "../../objects/lever/model/levercontactconditionsmodel.h"

#include "../../objects/lever/view/levercontactconditionsview.h"

// TODO: remove BEM
#include "../../objects/lever/bem/bemleverobject.h"

#include "../../utils/colorselectionwidget.h"

template <typename Graph>
AbstractPanelItem* addNewNodeToScene(PanelScene *s, ModeManager *)
{
    Graph *graph = new Graph;
    return graph;
}

void StandardPanelItemTypes::registerTypes(PanelItemFactory *factoryReg)
{
    {
        // Light Rect
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
        factory.nodeType = LightRectItem::ItemType;
        factory.prettyName = tr("Light Rect");
        factory.create = &addNewNodeToScene<LightRectItem>;
        factory.edit = [](AbstractPanelItem *item, ModeManager *mgr) -> QWidget*
        {
            LightRectItem *node = static_cast<LightRectItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Width, Height
            QDoubleSpinBox *wSpin = new QDoubleSpinBox;
            wSpin->setRange(0, 1000);
            wSpin->setDecimals(5);
            lay->addRow(tr("Width:"), wSpin);

            QDoubleSpinBox *hSpin = new QDoubleSpinBox;
            hSpin->setRange(0, 1000);
            hSpin->setDecimals(5);
            lay->addRow(tr("Height:"), hSpin);

            wSpin->setValue(node->rect().width());
            hSpin->setValue(node->rect().height());

            QObject::connect(node, &LightRectItem::rectChanged,
                    w, [wSpin, hSpin, node]()
            {
                wSpin->blockSignals(true);
                wSpin->setValue(node->rect().width());
                wSpin->blockSignals(false);

                hSpin->blockSignals(true);
                hSpin->setValue(node->rect().height());
                hSpin->blockSignals(false);
            });

            QObject::connect(wSpin, &QDoubleSpinBox::valueChanged,
                    node, [node](double newW)
            {
                QRectF r = node->rect();
                r.setWidth(newW);
                node->setRect(r);
            });

            QObject::connect(hSpin, &QDoubleSpinBox::valueChanged,
                    node, [node](double newH)
            {
                QRectF r = node->rect();
                r.setHeight(newH);
                node->setRect(r);
            });

            // Rotation
            QDoubleSpinBox *rSpin = new QDoubleSpinBox;
            rSpin->setRange(-180, +180);
            rSpin->setDecimals(5);
            lay->addRow(tr("Rotation:"), rSpin);

            rSpin->setValue(node->rotation());

            QObject::connect(node, &LightRectItem::rotationChanged,
                    w, [rSpin, node]()
            {
                rSpin->blockSignals(true);
                rSpin->setValue(node->rotation());
                rSpin->blockSignals(false);
            });

            QObject::connect(rSpin, &QDoubleSpinBox::valueChanged,
                    node, [node](double newR)
            {
                node->setRotation(newR);
            });

            // Light
            SimulationObjectLineEdit *lightEdit = new SimulationObjectLineEdit(mgr, {LightBulbObject::Type});
            QObject::connect(node, &LightRectItem::lightChanged,
                             lightEdit, [node, lightEdit]()
            {
                lightEdit->setObject(node->lightObject());
            });
            QObject::connect(lightEdit, &SimulationObjectLineEdit::objectChanged,
                             node, [node](AbstractSimulationObject *obj)
            {
                node->setLightObject(static_cast<LightBulbObject *>(obj));
            });

            lightEdit->setObject(node->lightObject());
            lay->addRow(tr("Light:"), lightEdit);

            // Color
            ColorSelectionWidget *colorW = new ColorSelectionWidget;
            colorW->setColor(node->color());

            QObject::connect(colorW, &ColorSelectionWidget::colorChanged,
                             node, [node, colorW]()
            {
                node->setColor(colorW->color());
            });
            QObject::connect(node, &LightRectItem::colorChanged,
                             colorW, [node, colorW]()
            {
                colorW->setColor(node->color());
            });
            lay->addRow(tr("Color:"), colorW);

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    /*
    {
        // Light Bulb node
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
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
        // ACEI Button
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
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
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
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
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
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
    */
}

