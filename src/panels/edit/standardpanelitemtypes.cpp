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

// Special items
#include "../graphs/lightrectitem.h"
#include "lightrectlightsmodel.h"
#include "lightrectlightsview.h"

#include "../graphs/imagepanelitem.h"

// Other items
#include "../graphs/aceibuttonpanelitem.h"
#include "../graphs/aceileverpanelitem.h"
#include "../graphs/aceilightpanelitem.h"
#include "../graphs/acesasibleverpanelitem.h"
#include "../graphs/bempanelitem.h"

#include <QWidget>
#include <QFormLayout>

#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

#include "../../views/modemanager.h"
#include "../../views/viewmanager.h"

#include "../../objects/simulationobjectlineedit.h"
#include "../../objects/simulationobjectfactory.h"

#include "../../objects/interfaces/buttoninterface.h"
#include "../../objects/interfaces/leverinterface.h"
#include "../../objects/interfaces/sasibaceleverextrainterface.h"

#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../objects/relais/model/abstractrelais.h"
#include "../../objects/lever/acei/aceileverobject.h"

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
        factory.edit = [](AbstractPanelItem *item, ViewManager *viewMgr) -> QWidget*
        {
            LightRectItem *node = static_cast<LightRectItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Width, Height
            QDoubleSpinBox *wSpin = new QDoubleSpinBox;
            wSpin->setRange(0, 99999);
            wSpin->setDecimals(5);
            lay->addRow(tr("Width:"), wSpin);

            QDoubleSpinBox *hSpin = new QDoubleSpinBox;
            hSpin->setRange(0, 99999);
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

            // Lights
            LightRectLightsView *lightsView = new LightRectLightsView;
            lay->addRow(lightsView);

            QObject::connect(node, &LightRectItem::lightsChanged,
                             lightsView, [node, lightsView]()
            {
                lightsView->loadFrom(node);
            });
            QObject::connect(lightsView, &LightRectLightsView::needsSave,
                             node, [node, lightsView]()
            {
                lightsView->saveTo(node);
            });

            lightsView->loadFrom(node);

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // Image
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
        factory.nodeType = ImagePanelItem::ItemType;
        factory.prettyName = tr("Image");
        factory.create = &addNewNodeToScene<ImagePanelItem>;
        factory.edit = [](AbstractPanelItem *item, ViewManager *viewMgr) -> QWidget*
        {
            ImagePanelItem *node = static_cast<ImagePanelItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Image File
            QLineEdit *imageFileEdit = new QLineEdit;
            lay->addRow(tr("Image File:"), imageFileEdit);

            QPushButton *browseBut = new QPushButton(tr("Browse"));
            lay->addRow(browseBut);

            QPushButton *applyBut = new QPushButton(tr("Apply"));
            lay->addRow(applyBut);

            imageFileEdit->setText(node->imageFileName());

            QObject::connect(applyBut, &QPushButton::clicked,
                             node, [node, imageFileEdit]()
            {
                node->setImageFileName(imageFileEdit->text());
            });

            QObject::connect(browseBut, &QPushButton::clicked,
                             imageFileEdit, [imageFileEdit]()
            {
                QString str = QFileDialog::getOpenFileName(imageFileEdit,
                                                           tr("Choose PNG Image"),
                                                           imageFileEdit->text());
                imageFileEdit->setText(str);
            });

            // Scale
            QDoubleSpinBox *scaleSpin = new QDoubleSpinBox;
            scaleSpin->setRange(20, 400);
            scaleSpin->setDecimals(2);
            lay->addRow(tr("Scale:"), scaleSpin);

            scaleSpin->setValue(node->imageScale() * 100.0);
            scaleSpin->setSuffix(tr("%"));

            QObject::connect(node, &ImagePanelItem::imageScaleChanged,
                             w, [scaleSpin, node]()
            {
                scaleSpin->blockSignals(true);
                scaleSpin->setValue(node->imageScale() * 100.0);
                scaleSpin->blockSignals(false);
            });

            QObject::connect(scaleSpin, &QDoubleSpinBox::valueChanged,
                             node, [node](double newScale)
            {
                node->setImageScale(newScale / 100.0);
            });

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // ACEI Button
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
        factory.nodeType = ACEIButtonPanelItem::ItemType;
        factory.prettyName = tr("ACEI Button");
        factory.create = &addNewNodeToScene<ACEIButtonPanelItem>;
        factory.edit = [](AbstractPanelItem *item, ViewManager *viewMgr) -> QWidget*
        {
            ACEIButtonPanelItem *buttonItem = static_cast<ACEIButtonPanelItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Button
            const QStringList buttonTypes = viewMgr->modeMgr()->objectFactory()
                    ->typesForInterface(ButtonInterface::IfaceType);

            SimulationObjectLineEdit *buttonEdit =
                    new SimulationObjectLineEdit(viewMgr, buttonTypes);

            QObject::connect(buttonItem, &ACEIButtonPanelItem::buttonChanged,
                             buttonEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(buttonEdit, &SimulationObjectLineEdit::objectChanged,
                             buttonItem, [buttonItem](AbstractSimulationObject *obj)
            {
                buttonItem->setButton(obj);
            });
            buttonEdit->setObject(buttonItem->button());

            lay->addRow(tr("Button:"), buttonEdit);

            // Lights
            SimulationObjectLineEdit *lightEdits[ACEIButtonPanelItem::NLights];
            ColorSelectionWidget *lightColorEdits[ACEIButtonPanelItem::NLights];
            const QString lightNames[ACEIButtonPanelItem::NLights] = {
                tr("Left light:"),
                tr("Central light:"),
                tr("Right light:")
            };
            const QString lightColorNames[ACEIButtonPanelItem::NLights] = {
                tr("Left color:"),
                tr("Central color:"),
                tr("Right color:")
            };

            for(int i = 0; i < ACEIButtonPanelItem::NLights; i++)
            {
                lightEdits[i] = new SimulationObjectLineEdit(viewMgr, {LightBulbObject::Type});
                QObject::connect(lightEdits[i], &SimulationObjectLineEdit::objectChanged,
                                 buttonItem, [buttonItem, i](AbstractSimulationObject *obj)
                {
                    buttonItem->setLight(ACEIButtonPanelItem::LightPosition(i),
                                         static_cast<LightBulbObject *>(obj));
                });

                lay->addRow(lightNames[i], lightEdits[i]);

                // Left Light Color
                lightColorEdits[i] = new ColorSelectionWidget;

                QObject::connect(lightColorEdits[i], &ColorSelectionWidget::colorChanged,
                                 buttonItem, [buttonItem, lightColorEdits, i]()
                {
                    buttonItem->setLightColor(ACEIButtonPanelItem::LightPosition(i),
                                              lightColorEdits[i]->color());
                });

                lay->addRow(lightColorNames[i], lightColorEdits[i]);
            }

            auto updateLights = [buttonItem, lightEdits, lightColorEdits]()
            {
                for(int i = 0; i < ACEIButtonPanelItem::NLights; i++)
                {
                    lightEdits[i]->setObject(buttonItem->getLight(ACEIButtonPanelItem::LightPosition(i)));
                    lightColorEdits[i]->setColor(buttonItem->getLightColor(ACEIButtonPanelItem::LightPosition(i)));
                }
            };

            QObject::connect(buttonItem, &ACEIButtonPanelItem::lightsChanged,
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
        factory.nodeType = ACEILeverPanelItem::ItemType;
        factory.prettyName = tr("ACEI Lever");
        factory.create = &addNewNodeToScene<ACEILeverPanelItem>;
        factory.edit = [](AbstractPanelItem *item, ViewManager *viewMgr) -> QWidget*
        {
            ACEILeverPanelItem *leverItem = static_cast<ACEILeverPanelItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Lever
            // TODO: remove BEM
            SimulationObjectLineEdit *leverEdit = new SimulationObjectLineEdit(viewMgr, {ACEILeverObject::Type, BEMLeverObject::Type});
            QObject::connect(leverItem, &ACEILeverPanelItem::leverChanged,
                             leverEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(leverEdit, &SimulationObjectLineEdit::objectChanged,
                             leverItem, [leverItem](AbstractSimulationObject *obj)
            {
                leverItem->setLever(obj);
            });
            leverEdit->setObject(leverItem->lever());

            lay->addRow(tr("Lever:"), leverEdit);

            // Lights
            SimulationObjectLineEdit *lightEdits[ACEILeverPanelItem::NLights];
            ColorSelectionWidget *lightColorEdits[ACEILeverPanelItem::NLights];
            const QString lightNames[ACEILeverPanelItem::NLights] = {
                tr("Left light:"),
                tr("Central light:"),
                tr("Right light:")
            };
            const QString lightColorNames[ACEILeverPanelItem::NLights] = {
                tr("Left color:"),
                tr("Central color:"),
                tr("Right color:")
            };

            for(int i = 0; i < ACEILeverPanelItem::NLights; i++)
            {
                lightEdits[i] = new SimulationObjectLineEdit(viewMgr, {LightBulbObject::Type});
                QObject::connect(lightEdits[i], &SimulationObjectLineEdit::objectChanged,
                                 leverItem, [leverItem, i](AbstractSimulationObject *obj)
                {
                    leverItem->setLight(ACEILeverPanelItem::LightPosition(i),
                                        static_cast<LightBulbObject *>(obj));
                });

                lay->addRow(lightNames[i], lightEdits[i]);

                // Light Color
                lightColorEdits[i] = new ColorSelectionWidget;

                QObject::connect(lightColorEdits[i], &ColorSelectionWidget::colorChanged,
                                 leverItem, [leverItem, lightColorEdits, i]()
                {
                    leverItem->setLightColor(ACEILeverPanelItem::LightPosition(i),
                                             lightColorEdits[i]->color());
                });

                lay->addRow(lightColorNames[i], lightColorEdits[i]);
            }

            auto updateLights = [leverItem, lightEdits, lightColorEdits]()
            {
                for(int i = 0; i < ACEILeverPanelItem::NLights; i++)
                {
                    lightEdits[i]->setObject(leverItem->getLight(ACEILeverPanelItem::LightPosition(i)));
                    lightColorEdits[i]->setColor(leverItem->getLightColor(ACEILeverPanelItem::LightPosition(i)));
                }
            };

            QObject::connect(leverItem, &ACEILeverPanelItem::lightsChanged,
                             w, updateLights);

            updateLights();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // ACEI Light
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
        factory.nodeType = ACEILightPanelItem::ItemType;
        factory.prettyName = tr("ACEI Light");
        factory.create = &addNewNodeToScene<ACEILightPanelItem>;
        factory.edit = [](AbstractPanelItem *item, ViewManager *viewMgr) -> QWidget*
        {
            ACEILightPanelItem *lightItem = static_cast<ACEILightPanelItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Lights
            SimulationObjectLineEdit *lightEdit = nullptr;
            ColorSelectionWidget *lightColorEdit = nullptr;

            lightEdit = new SimulationObjectLineEdit(viewMgr, {LightBulbObject::Type});
            QObject::connect(lightEdit, &SimulationObjectLineEdit::objectChanged,
                             lightItem, [lightItem](AbstractSimulationObject *obj)
            {
                lightItem->setLight(static_cast<LightBulbObject *>(obj));
            });

            lay->addRow(tr("Light:"), lightEdit);

            // Light Color
            lightColorEdit = new ColorSelectionWidget;

            QObject::connect(lightColorEdit, &ColorSelectionWidget::colorChanged,
                             lightItem, [lightItem, lightColorEdit]()
            {
                lightItem->setLightColor(lightColorEdit->color());
            });

            lay->addRow(tr("Color:"), lightColorEdit);

            auto updateLights = [lightItem, lightEdit, lightColorEdit]()
            {
                lightEdit->setObject(lightItem->getLight());
                lightColorEdit->setColor(lightItem->getLightColor());
            };

            QObject::connect(lightItem, &ACEILightPanelItem::lightsChanged,
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
        factory.nodeType = ACESasibLeverPanelItem::ItemType;
        factory.prettyName = tr("ACE Sasib Lever");
        factory.create = &addNewNodeToScene<ACESasibLeverPanelItem>;
        factory.edit = [](AbstractPanelItem *item, ViewManager *viewMgr) -> QWidget*
        {
            ACESasibLeverPanelItem *leverItem = static_cast<ACESasibLeverPanelItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Lever
            const QStringList sasibTypes = viewMgr->modeMgr()->objectFactory()
                    ->typesForInterface(SasibACELeverExtraInterface::IfaceType);

            SimulationObjectLineEdit *leverEdit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        sasibTypes);

            QObject::connect(leverItem, &ACESasibLeverPanelItem::leverChanged,
                             leverEdit, &SimulationObjectLineEdit::setObject);
            QObject::connect(leverEdit, &SimulationObjectLineEdit::objectChanged,
                             leverItem, [leverItem](AbstractSimulationObject *obj)
            {
                leverItem->setLever(obj);
            });
            leverEdit->setObject(leverItem->lever());

            lay->addRow(tr("Lever:"), leverEdit);

            // Button
            SimulationObjectLineEdit *buttonEdits[ACESasibLeverPanelItem::NLights];
            const QString buttonNames[ACESasibLeverPanelItem::NLights] = {
                tr("Left button:"),
                tr("Right button:")
            };

            const QStringList buttonTypes = viewMgr->modeMgr()->objectFactory()
                    ->typesForInterface(ButtonInterface::IfaceType);

            for(int i = 0; i < ACESasibLeverPanelItem::NLights; i++)
            {
                buttonEdits[i] = new SimulationObjectLineEdit(viewMgr, buttonTypes);
                QObject::connect(buttonEdits[i], &SimulationObjectLineEdit::objectChanged,
                                 leverItem, [leverItem, i](AbstractSimulationObject *obj)
                {
                    leverItem->setButton(ACESasibLeverPanelItem::LightPosition(i),
                                         obj);
                });

                lay->addRow(buttonNames[i], buttonEdits[i]);
            }

            auto updateButtons = [leverItem, buttonEdits]()
            {
                for(int i = 0; i < ACESasibLeverPanelItem::NLights; i++)
                {
                    buttonEdits[i]->setObject(leverItem->getButton(ACESasibLeverPanelItem::LightPosition(i)));
                }
            };

            QObject::connect(leverItem, &ACESasibLeverPanelItem::buttonsChanged,
                             w, updateButtons);
            updateButtons();

            // Lights
            SimulationObjectLineEdit *lightEdits[ACESasibLeverPanelItem::NLights];
            ColorSelectionWidget *lightColorEdits[ACESasibLeverPanelItem::NLights];
            const QString lightNames[ACESasibLeverPanelItem::NLights] = {
                tr("Left light:"),
                tr("Right light:")
            };
            const QString lightColorNames[ACESasibLeverPanelItem::NLights] = {
                tr("Left color:"),
                tr("Right color:")
            };

            for(int i = 0; i < ACESasibLeverPanelItem::NLights; i++)
            {
                lightEdits[i] = new SimulationObjectLineEdit(viewMgr, {LightBulbObject::Type});
                QObject::connect(lightEdits[i], &SimulationObjectLineEdit::objectChanged,
                                 leverItem, [leverItem, i](AbstractSimulationObject *obj)
                {
                    leverItem->setLight(ACESasibLeverPanelItem::LightPosition(i),
                                        static_cast<LightBulbObject *>(obj));
                });

                lay->addRow(lightNames[i], lightEdits[i]);

                // Light Color
                lightColorEdits[i] = new ColorSelectionWidget;

                QObject::connect(lightColorEdits[i], &ColorSelectionWidget::colorChanged,
                                 leverItem, [leverItem, lightColorEdits, i]()
                {
                    leverItem->setLightColor(ACESasibLeverPanelItem::LightPosition(i),
                                             lightColorEdits[i]->color());
                });

                lay->addRow(lightColorNames[i], lightColorEdits[i]);
            }

            auto updateLights = [leverItem, lightEdits, lightColorEdits]()
            {
                for(int i = 0; i < ACESasibLeverPanelItem::NLights; i++)
                {
                    lightEdits[i]->setObject(leverItem->getLight(ACESasibLeverPanelItem::LightPosition(i)));
                    lightColorEdits[i]->setColor(leverItem->getLightColor(ACESasibLeverPanelItem::LightPosition(i)));
                }
            };

            QObject::connect(leverItem, &ACESasibLeverPanelItem::lightsChanged,
                             w, updateLights);

            updateLights();

            return w;
        };

        factoryReg->registerFactory(factory);
    }

    {
        // BEM
        PanelItemFactory::FactoryItem factory;
        factory.needsName = PanelItemFactory::NeedsName::Never;
        factory.nodeType = BEMPanelItem::ItemType;
        factory.prettyName = tr("BEM Case");
        factory.create = &addNewNodeToScene<BEMPanelItem>;
        factory.edit = [](AbstractPanelItem *item, ViewManager *viewMgr) -> QWidget*
        {
            BEMPanelItem *specialItem = static_cast<BEMPanelItem *>(item);

            QWidget *w = new QWidget;
            QFormLayout *lay = new QFormLayout(w);

            // Consensus Lever
            SimulationObjectLineEdit *leverEdit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        {BEMLeverObject::Type});
            QObject::connect(leverEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setConsensusLever(static_cast<BEMLeverObject *>(obj));
            });
            lay->addRow(tr("Consensus Lever:"), leverEdit);

            SimulationObjectLineEdit *txButEdit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        viewMgr->modeMgr()->objectFactory()->typesForInterface(ButtonInterface::IfaceType));
            QObject::connect(txButEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setTxButton(obj);
            });
            lay->addRow(tr("Tx Button:"), txButEdit);

            SimulationObjectLineEdit *lightButEdit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        viewMgr->modeMgr()->objectFactory()->typesForInterface(ButtonInterface::IfaceType));
            QObject::connect(lightButEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setLightButton(obj);
            });
            lay->addRow(tr("Light Button:"), lightButEdit);

            SimulationObjectLineEdit *lightEdit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        {LightBulbObject::Type});
            QObject::connect(lightEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setLight(static_cast<LightBulbObject *>(obj));
            });
            lay->addRow(tr("Light:"), lightEdit);

            SimulationObjectLineEdit *R1Edit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        {AbstractRelais::Type});
            QObject::connect(R1Edit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setR1Relay(static_cast<AbstractRelais *>(obj));
            });
            lay->addRow(tr("R1:"), R1Edit);

            SimulationObjectLineEdit *C1Edit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        {AbstractRelais::Type});
            QObject::connect(C1Edit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setC1Relay(static_cast<AbstractRelais *>(obj));
            });
            lay->addRow(tr("C1:"), C1Edit);

            SimulationObjectLineEdit *HEdit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        {AbstractRelais::Type});
            QObject::connect(HEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setOccupancyRelay(static_cast<AbstractRelais *>(obj));
            });
            lay->addRow(tr("H:"), HEdit);

            SimulationObjectLineEdit *KEdit =
                    new SimulationObjectLineEdit(
                        viewMgr,
                        {AbstractRelais::Type});
            QObject::connect(KEdit, &SimulationObjectLineEdit::objectChanged,
                             specialItem, [specialItem](AbstractSimulationObject *obj)
            {
                specialItem->setKConditionsRelay(static_cast<AbstractRelais *>(obj));
            });
            lay->addRow(tr("K:"), KEdit);

            auto updSettings =
                    [specialItem, leverEdit, txButEdit,
                    lightButEdit, lightEdit,
                    R1Edit, C1Edit, HEdit, KEdit]()
            {
                leverEdit->setObject(specialItem->getConsensusLever());
                txButEdit->setObject(specialItem->getTxButton());
                lightButEdit->setObject(specialItem->getLightButton());
                lightEdit->setObject(specialItem->getLight());

                R1Edit->setObject(specialItem->getR1Relay());
                C1Edit->setObject(specialItem->getC1Relay());
                HEdit->setObject(specialItem->getOccupancyRelay());
                KEdit->setObject(specialItem->getKConditionsRelay());
            };

            QObject::connect(specialItem, &BEMPanelItem::settingsChanged,
                             w, updSettings);

            updSettings();

            return w;
        };

        factoryReg->registerFactory(factory);
    }


}

