/**
 * src/objects/interfaces/mechanical/view/genericmechanicaloptionswidget.cpp
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

#include "genericmechanicaloptionswidget.h"

#include "../../../abstractsimulationobject.h"
#include "../../mechanicalinterface.h"

#include "../model/mechanicalconditionsmodel.h"
#include "mechanicalconditionsview.h"

#include "../../../../utils/enumvaluesmodel.h"

#include <QFormLayout>
#include <QComboBox>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>

GenericMechanicalOptionsWidget::GenericMechanicalOptionsWidget(ModeManager *mgr,
                                                               MechanicalInterface *iface,
                                                               QWidget *parent)
    : QWidget{parent}
    , mMechanicalIface(iface)
{
    QFormLayout *lay = new QFormLayout(this);

    // Normal position and range
    mMinPosModel = new EnumValuesModel(this);
    mMinPosModel->setEnumDescFull(mMechanicalIface->positionDesc(), true);

    mMaxPosModel = new EnumValuesModel(this);
    mMaxPosModel->setEnumDescFull(mMechanicalIface->positionDesc(), true);

    mNormalPosModel = new EnumValuesModel(this);
    mNormalPosModel->setEnumDescFull(mMechanicalIface->positionDesc(), true);

    mMinPosCombo = new QComboBox;
    mMinPosCombo->setModel(mMinPosModel);
    lay->addRow(tr("Minimum Position:"), mMinPosCombo);

    mMaxPosCombo = new QComboBox;
    mMaxPosCombo->setModel(mMaxPosModel);
    lay->addRow(tr("Maximum Position:"), mMaxPosCombo);

    // TODO
    //mMinPosCombo->setVisible(mMechanicalIface->canChangeRange());
    //mMaxPosCombo->setVisible(mMechanicalIface->canChangeRange());

    mNormalPosCombo = new QComboBox;
    mNormalPosCombo->setModel(mNormalPosModel);
    mNormalPosCombo->setToolTip(tr("On program start,"
                                   " lever will be in this position."));
    lay->addRow(tr("Normal Position:"), mNormalPosCombo);

    connect(mMechanicalIface->object(), &AbstractSimulationObject::settingsChanged,
            this, &GenericMechanicalOptionsWidget::updatePositionRanges);

    updatePositionRanges();

    connect(mMinPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change min
        mMechanicalIface->setAbsoluteRange(mMinPosModel->valueAt(idx),
                                 mMechanicalIface->absoluteMax());
    });

    connect(mMaxPosCombo, &QComboBox::activated,
            this, [this](int idx)
    {
        // Change max
        mMechanicalIface->setAbsoluteRange(mMechanicalIface->absoluteMin(),
                                 mMaxPosModel->valueAt(idx));
    });

    // connect(mNormalPosCombo, &QComboBox::activated,
    //         this, [this](int idx)
    // {
    //     // Change normal position
    //     mMechanicalIface->setNormalPosition(mNormalPosModel->valueAt(idx));
    // });
    mNormalPosCombo->hide();

    lay->addRow(new QLabel(tr("Mechanical Conditions:")));

    applyConditionsBut = new QPushButton(tr("Apply"));
    lay->addRow(applyConditionsBut);

    tabWidget = new QTabWidget;
    lay->addRow(tabWidget);

    const int condCount = mMechanicalIface->getConditionsSetsCount();
    for(int i = 0; i < condCount; i++)
    {
        const auto cond = mMechanicalIface->getConditionSet(i);
        ConditionsView item;
        item.title = cond.title;
        item.model = new MechanicalConditionsModel(this);
        item.model->setConditionTree(cond.conditions.rootCondition);
        item.view = new MechanicalConditionsView(mgr);
        item.view->setModel(item.model);

        tabWidget->addTab(item.view, item.title);

        connect(item.model, &MechanicalConditionsModel::changed,
                this, [this]()
        {
            applyConditionsBut->setVisible(true);
        });
    }

    tabWidget->setMinimumSize(300, 400);

    applyConditionsBut->setVisible(false);
    connect(applyConditionsBut, &QPushButton::clicked,
            this, &GenericMechanicalOptionsWidget::applyConditions);

    connect(mMechanicalIface->object(), &AbstractSimulationObject::interfacePropertyChanged,
            this, &GenericMechanicalOptionsWidget::onInterfacePropertyChanged);
}

void GenericMechanicalOptionsWidget::updatePositionRanges()
{
    int min = mMechanicalIface->absoluteMin();
    int max = mMechanicalIface->absoluteMax();

    bool updateMin = false;
    bool updateMax = false;

    if(min != minPos)
    {
        minPos = min;
        updateMin = true;
    }

    if(max != maxPos)
    {
        maxPos = max;
        updateMax = true;
    }

    if(updateMin)
    {
        const int posIdx = mMinPosModel->rowForValue(minPos);
        if(mMinPosCombo->currentIndex() != posIdx)
            mMinPosCombo->setCurrentIndex(posIdx);

        // Allow setting Max to at least Min position
        mMaxPosModel->setEnumDescRange(mMaxPosModel->enumDesc(), true,
                                       minPos,
                                       mMechanicalIface->positionDesc().maxValue);
    }

    if(updateMin || updateMax)
    {
        // Max positions could have changed index, update it
        const int maxPosIdx = mMaxPosModel->rowForValue(maxPos);
        if(mMaxPosCombo->currentIndex() != maxPosIdx)
            mMaxPosCombo->setCurrentIndex(maxPosIdx);

        // Normal position must be in range min/max
        mNormalPosModel->setEnumDescRange(mNormalPosModel->enumDesc(), true,
                                          minPos, maxPos);
    }

    // const int normalPosIdx = mNormalPosModel->rowForValue(mMechanicalIface->normalPosition());
    // if(normalPosIdx != -1 && mNormalPosCombo->currentIndex() != normalPosIdx)
    //     mNormalPosCombo->setCurrentIndex(normalPosIdx);
}

void GenericMechanicalOptionsWidget::applyConditions()
{
    for(int i = 0; i < mConditionViews.size(); i++)
    {
        const ConditionsView& item = mConditionViews.at(i);
        MechanicalCondition c = item.model->getConditionTree();
        mMechanicalIface->setConditionSetConditions(i, c);
    }

    applyConditionsBut->setVisible(false);
}

void GenericMechanicalOptionsWidget::onInterfacePropertyChanged(const QString &ifaceName,
                                                                const QString &propName,
                                                                const QVariant &value)
{
    if(ifaceName == MechanicalInterface::IfaceType)
    {
        if(!mMechanicalIface)
            return;

        if(propName == MechanicalInterface::MecConditionsPropName)
        {
            int i = value.toInt();

            if(i < 0 || i >= mConditionViews.size())
                return;

            const auto cond = mMechanicalIface->getConditionSet(i);
            ConditionsView& item = mConditionViews[i];

            item.title = cond.title;
            tabWidget->setTabText(i, item.title);

            item.model->setConditionTree(cond.conditions.rootCondition);
            item.view->expandAll();

            return;
        }
    }
}
