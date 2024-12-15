/**
 * src/panels/edit/panelitemfactory.cpp
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

#include "panelitemfactory.h"

#include "../panelscene.h"
#include "../abstractpanelitem.h"

#include "../view/panellistmodel.h"

#include <QDialog>
#include <QPointer>

#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>

#include <QMessageBox>

PanelItemFactory::PanelItemFactory(QObject *parent)
    : QObject(parent)
{

}

QStringList PanelItemFactory::getRegisteredTypes() const
{
    QStringList result;
    result.reserve(mItems.size());
    for(const FactoryItem& item : std::as_const(mItems))
        result.append(item.nodeType);
    return result;
}

AbstractPanelItem *PanelItemFactory::createItem(const QString &nodeType,
                                                PanelScene *scene)
{
    const FactoryItem *factory = getItemForType(nodeType);
    if(!factory)
        return nullptr;

    AbstractPanelItem *item = factory->create(scene,
                                              scene->panelsModel()->modeMgr());
    return item;
}

void PanelItemFactory::editItem(QWidget *parent, AbstractPanelItem *item)
{
    const FactoryItem *factory = getItemForType(item->itemType());
    if(!factory)
        return;

    QPointer<QDialog> dlg = new QDialog(parent);
    QFormLayout *lay = new QFormLayout(dlg);

    dlg->setWindowTitle(tr("Edit %1").arg(item->objectName()));

    // Type
    lay->addRow(tr("Type"), new QLabel(prettyName(item->itemType())));

    // Name
    if(factory->needsName != NeedsName::Never)
    {
        QLineEdit *nameEdit = new QLineEdit(dlg);
        connect(item, &QObject::objectNameChanged,
                nameEdit, [nameEdit](const QString& name)
        {
            if(nameEdit->text() != name)
                nameEdit->setText(name);
        });

        connect(nameEdit, &QLineEdit::textEdited,
                item, [item](const QString& name)
        {
            item->setObjectName(name);
        });

        nameEdit->setText(item->objectName());

        lay->addRow(tr("Name"), nameEdit);
    }

    // Position
    QDoubleSpinBox *xSpin = new QDoubleSpinBox;
    xSpin->setRange(std::numeric_limits<double>::min(),
                   std::numeric_limits<double>::max());
    xSpin->setDecimals(5);
    lay->addRow(tr("X:"), xSpin);

    QDoubleSpinBox *ySpin = new QDoubleSpinBox;
    ySpin->setRange(std::numeric_limits<double>::min(),
                   std::numeric_limits<double>::max());
    ySpin->setDecimals(5);
    lay->addRow(tr("Y:"), ySpin);

    xSpin->setValue(item->x());
    ySpin->setValue(item->y());

    connect(item, &AbstractPanelItem::xChanged,
            dlg, [xSpin, item]()
    {
        xSpin->blockSignals(true);
        xSpin->setValue(item->x());
        xSpin->blockSignals(false);
    });
    connect(item, &AbstractPanelItem::yChanged,
            dlg, [ySpin, item]()
    {
        ySpin->blockSignals(true);
        ySpin->setValue(item->y());
        ySpin->blockSignals(false);
    });

    connect(xSpin, &QDoubleSpinBox::valueChanged,
            item, [item](double newX)
    {
        item->setX(newX);
    });
    connect(ySpin, &QDoubleSpinBox::valueChanged,
            item, [item](double newY)
    {
        item->setY(newY);
    });

    if(factory->edit)
    {
        QWidget *customWidget = factory->edit(item,
                                              item->panelScene()->panelsModel()->modeMgr());
        if(customWidget)
            lay->addWidget(customWidget);
    }

    QPushButton *delBut = new QPushButton(tr("Delete"));
    delBut->setAutoDefault(false);
    delBut->setDefault(false);

    connect(delBut, &QPushButton::clicked,
            dlg, [dlg, item]()
    {
        int ret = QMessageBox::question(dlg,
                                        tr("Delete Item?"),
                                        tr("Are you sure?"));
        if(ret == QMessageBox::Yes)
        {
            delete dlg;

            // Remove item
            auto *scene = item->panelScene();
            if(scene)
                scene->removeNode(item);
        }
    });

    QPushButton *okBut = new QPushButton(tr("Ok"));
    connect(okBut, &QPushButton::clicked, dlg, &QDialog::accept);
    okBut->setFocus();

    okBut->setAutoDefault(false);
    okBut->setDefault(false);

    QHBoxLayout *butLay = new QHBoxLayout;
    butLay->addWidget(delBut);
    butLay->addWidget(okBut);
    lay->setLayout(lay->rowCount(), QFormLayout::SpanningRole, butLay);

    dlg->exec();

    if(dlg)
        delete dlg;
}

QString PanelItemFactory::prettyName(const QString &nodeType) const
{
    const FactoryItem *factory = getItemForType(nodeType);
    if(!factory)
        return QString();

    return factory->prettyName;
}

PanelItemFactory::NeedsName PanelItemFactory::needsName(const QString &nodeType) const
{
    const FactoryItem *factory = getItemForType(nodeType);
    if(!factory)
        return NeedsName::Never;

    return factory->needsName;
}

void PanelItemFactory::registerFactory(const FactoryItem &factory)
{
    mItems.append(factory);
}

const PanelItemFactory::FactoryItem *PanelItemFactory::getItemForType(const QString &nodeType) const
{
    for(const FactoryItem& item : std::as_const(mItems))
    {
        if(item.nodeType == nodeType)
            return &item;
    }
    return nullptr;
}
