/**
 * src/circuits/edit/nodeeditfactory.cpp
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

#include "nodeeditfactory.h"

#include "../circuitscene.h"
#include "../nodes/abstractcircuitnode.h"
#include "../graphs/abstractnodegraphitem.h"
#include "../graphs/cablegraphitem.h"

#include "../view/circuitlistmodel.h"

#include <QDialog>
#include <QPointer>

#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include <QMessageBox>

NodeEditFactory::NodeEditFactory(QObject *parent)
    : QObject(parent)
{

}

QStringList NodeEditFactory::getRegisteredTypes() const
{
    QStringList result;
    result.reserve(mItems.size());
    for(const FactoryItem& item : std::as_const(mItems))
        result.append(item.nodeType);
    return result;
}

AbstractNodeGraphItem *NodeEditFactory::createItem(const QString &nodeType,
                                                   CircuitScene *scene)
{
    const FactoryItem *factory = getItemForType(nodeType);
    if(!factory)
        return nullptr;

    AbstractNodeGraphItem *item = factory->create(scene,
                                                  scene->circuitsModel()->modeMgr());
    return item;
}

void NodeEditFactory::editItem(QWidget *parent, AbstractNodeGraphItem *item)
{
    const FactoryItem *factory = getItemForType(item->getAbstractNode()->nodeType());
    if(!factory)
        return;

    AbstractCircuitNode *node = item->getAbstractNode();

    QPointer<QDialog> dlg = new QDialog(parent);
    QFormLayout *lay = new QFormLayout(dlg);

    dlg->setWindowTitle(tr("Edit %1").arg(node->objectName()));

    // Type
    lay->addRow(tr("Type"), new QLabel(prettyName(node->nodeType())));

    // Name
    if(factory->needsName != NeedsName::Never)
    {
        QLineEdit *nameEdit = new QLineEdit(dlg);
        connect(node, &QObject::objectNameChanged,
                nameEdit, [nameEdit](const QString& name)
        {
            if(nameEdit->text() != name)
                nameEdit->setText(name);
        });

        connect(nameEdit, &QLineEdit::textEdited,
                node, [node](const QString& name)
        {
            node->setObjectName(name);
        });

        nameEdit->setText(node->objectName());

        lay->addRow(tr("Name"), nameEdit);
    }

    if(factory->edit)
    {
        QWidget *customWidget = factory->edit(item,
                                              item->circuitScene()->circuitsModel()->modeMgr());
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
            auto *scene = item->circuitScene();
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

void NodeEditFactory::editCable(QWidget *parent, CableGraphItem *item)
{
    QPointer<QDialog> dlg = new QDialog(parent);
    QFormLayout *lay = new QFormLayout(dlg);

    dlg->setWindowTitle(tr("Edit Cable"));

    QPushButton *editBut = new QPushButton(tr("Edit Path"));
    lay->addWidget(editBut);

    editBut->setFocus();
    editBut->setDefault(true);

    connect(editBut, &QPushButton::clicked,
            dlg, [dlg, item]()
    {
        dlg->accept();
        delete dlg;

        // Edit item
        auto *scene = item->circuitScene();
        if(scene)
            scene->startEditCable(item);
    });


    QPushButton *delBut = new QPushButton(tr("Delete"));
    delBut->setDefault(false);
    lay->addWidget(delBut);

    connect(delBut, &QPushButton::clicked,
            dlg, [dlg, item]()
    {
        int ret = QMessageBox::question(dlg,
                                        tr("Delete Item?"),
                                        tr("Are you sure?"));
        if(ret == QMessageBox::Yes)
        {
            dlg->accept();
            delete dlg;

            // Remove item
            auto *scene = item->circuitScene();
            if(scene)
                scene->removeCable(item->cable());
        }
    });

    dlg->exec();

    if(dlg)
        delete dlg;
}

QString NodeEditFactory::prettyName(const QString &nodeType) const
{
    const FactoryItem *factory = getItemForType(nodeType);
    if(!factory)
        return QString();

    return factory->prettyName;
}

NodeEditFactory::NeedsName NodeEditFactory::needsName(const QString &nodeType) const
{
    const FactoryItem *factory = getItemForType(nodeType);
    if(!factory)
        return NeedsName::Never;

    return factory->needsName;
}

void NodeEditFactory::registerFactory(const FactoryItem &factory)
{
    mItems.append(factory);
}

const NodeEditFactory::FactoryItem *NodeEditFactory::getItemForType(const QString &nodeType) const
{
    for(const FactoryItem& item : std::as_const(mItems))
    {
        if(item.nodeType == nodeType)
            return &item;
    }
    return nullptr;
}
