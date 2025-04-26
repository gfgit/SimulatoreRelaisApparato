/**
 * src/utils/itemobjectreplacedlg_impl.hpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#ifndef ITEM_OBJECT_REPLACE_DLG_IMPL_HPP
#define ITEM_OBJECT_REPLACE_DLG_IMPL_HPP

#include "itemobjectreplacedlg.h"

#include "objectproperty.h"
#include "jsondiff.h"
#include <QJsonObject>

#include "../objects/simulationobjectlineedit.h"
#include "../objects/simulationobjectfactory.h"
#include "../objects/abstractsimulationobject.h"
#include "../objects/abstractsimulationobjectmodel.h"

#include "../views/modemanager.h"
#include "../views/viewmanager.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include <QPushButton>
#include <QLineEdit>

#include <QDialogButtonBox>

#include <QScrollArea>

#include <QCoreApplication>

class ItemObjectReplaceDlg_tr
{
    Q_DECLARE_TR_FUNCTIONS(ItemObjectReplaceDlg_tr)
};

template<typename NodeTraits>
ItemObjectReplaceDlg<NodeTraits>::ItemObjectReplaceDlg(ViewManager *viewMgr,
                                                       const QVector<Node *> &items,
                                                       QWidget *parent)
    : QDialog{parent}
    , mViewMgr(viewMgr)
    , mItems(items)
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);

    QPushButton *replaceStrBut = new QPushButton(ItemObjectReplaceDlg_tr::tr("Replace Name String"));
    mainLay->addWidget(replaceStrBut);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    mainLay->addWidget(scrollArea);

    QWidget *viewport = new QWidget;
    scrollArea->setWidget(viewport);
    mGroupsLay = new QVBoxLayout(viewport);

    QDialogButtonBox *dlgBut = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                    Qt::Horizontal);
    mainLay->addWidget(dlgBut);

    connect(dlgBut, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(dlgBut, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    createGroups();
    reloadGroups();

    connect(replaceStrBut, &QPushButton::clicked,
            this, [this]()
    {
        onReplaceName();
    });

    setWindowTitle(ItemObjectReplaceDlg_tr::tr("Batch Object Replace"));
}

template<typename NodeTraits>
void ItemObjectReplaceDlg<NodeTraits>::replaceName(const QString &oldStr, const QString &newStr)
{
    for(NodeGroupEditWidget_ *w : mGroups)
    {
        w->replaceName(mViewMgr, oldStr, newStr);
    }

    saveChanges();
    reloadGroups();
}

template<typename NodeTraits>
void ItemObjectReplaceDlg<NodeTraits>::done(int result)
{
    if(result == QDialog::Accepted)
        saveChanges();

    QDialog::done(result);
}

template<typename NodeTraits>
void ItemObjectReplaceDlg<NodeTraits>::reloadGroups()
{
    for(NodeGroupEditWidget_ *w : mGroups)
    {
        w->reloadGroup(mViewMgr);
    }
}

template<typename NodeTraits>
void ItemObjectReplaceDlg<NodeTraits>::createGroups()
{
    QHash<QString, QVector<Node *>> groups;
    QStringList ignoreTypes;

    for(Node *item : mItems)
    {
        const QString nodeType = NodeTraits::getNodeType(item);
        if(ignoreTypes.contains(nodeType))
            continue;

        auto it = groups.find(nodeType);
        if(it != groups.end())
        {
            it.value().append(item);
            continue;
        }

        QVector<ObjectProperty> objProps;
        NodeTraits::getObjectProperties(item, objProps);
        if(objProps.isEmpty())
        {
            ignoreTypes.append(nodeType);
            continue;
        }

        groups.insert(nodeType, {item});
    }

    ModeManager *modeMgr = mViewMgr->modeMgr();
    const QStringList nodeTypes = NodeTraits::getRegisteredTypes(modeMgr);

    for(const QString& nodeType : nodeTypes)
    {
        auto it = groups.find(nodeType);
        if(it == groups.end())
            continue;

        NodeGroupEditWidget_ *w = new NodeGroupEditWidget_(it.value());
        w->createGroup(mViewMgr);

        w->setTitle(NodeTraits::prettyTypeName(modeMgr, nodeType));
        mGroups.append(w);
        mGroupsLay->addWidget(w);
    }
}

template<typename NodeTraits>
void ItemObjectReplaceDlg<NodeTraits>::saveChanges()
{
    for(NodeGroupEditWidget_ *w : mGroups)
    {
        w->saveChanges(mViewMgr);
    }
}

template<typename NodeTraits>
void ItemObjectReplaceDlg<NodeTraits>::onReplaceName()
{
    QPointer<QDialog> dlg = new QDialog(this);

    QVBoxLayout *lay = new QVBoxLayout(dlg);

    lay->addWidget(new QLabel(ItemObjectReplaceDlg_tr::tr("Replace a string in object names.\n"
                                 "If an object with new name exists it replaces the old one.\n"
                                 "If not, it's created by cloning the old one.")));

    QLineEdit *oldEdit = new QLineEdit;
    oldEdit->setPlaceholderText(ItemObjectReplaceDlg_tr::tr("Old string"));
    lay->addWidget(oldEdit);

    QLineEdit *newEdit = new QLineEdit;
    newEdit->setPlaceholderText(ItemObjectReplaceDlg_tr::tr("New string"));
    lay->addWidget(newEdit);

    QDialogButtonBox *dlgBut = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                    Qt::Horizontal);
    lay->addWidget(dlgBut);

    connect(dlgBut, &QDialogButtonBox::accepted,
            dlg, &QDialog::accept);
    connect(dlgBut, &QDialogButtonBox::rejected,
            dlg, &QDialog::reject);

    const int ret = dlg->exec();
    if(!dlg || ret != QDialog::Accepted)
        return;

    const QString oldStr = oldEdit->text();
    const QString newStr = newEdit->text();

    if(oldStr.isEmpty() || newStr == oldStr)
        return;

    replaceName(oldStr, newStr);
}

template<typename NodeTraits>
void ItemObjectReplaceDlg<NodeTraits>::batchNodeEdit(const QVector<Node *> &items,
                                                     ViewManager *viewMgr,
                                                     QWidget *parent)
{
    Node *curItem = items.first();

    QJsonObject origSettings;
    NodeTraits::saveToJSON(curItem, origSettings);

    // Edit first item, do not allow delete
    NodeTraits::editItem(curItem, viewMgr, parent);

    QJsonObject newSettings;
    NodeTraits::saveToJSON(curItem, newSettings);

    JSONDiff::checkDifferences(origSettings, newSettings);

    const QStringList ignoreKeys{"x", "y", "name", "rotation",
                                 "text_rotation", "type"};

    for(const QString& key : ignoreKeys)
    {
        newSettings.remove(key);
    }

    if(newSettings.isEmpty())
        return;

    for(Node *item : std::as_const(items))
    {
        if(item == curItem)
            continue;

        QJsonObject settings;
        NodeTraits::saveToJSON(item, settings);

        JSONDiff::applyDiffSub(settings, newSettings);

        NodeTraits::loadFromJSON(item, settings, viewMgr->modeMgr());
    }

    viewMgr->modeMgr()->setFileEdited();
}

template<typename NodeTraits>
NodeGroupEditWidget<NodeTraits>::NodeGroupEditWidget(const QVector<Node *> &items)
    : QGroupBox()
    , mItems(items)
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);
}

template<typename NodeTraits>
void NodeGroupEditWidget<NodeTraits>::createGroup(ViewManager *viewMgr)
{
    QVBoxLayout *mainLay = static_cast<QVBoxLayout *>(layout());

    QVector<ObjectProperty> objProps;
    NodeTraits::getObjectProperties(mItems.first(), objProps);

    for(const ObjectProperty& prop : std::as_const(objProps))
    {
        PropertyEntry entry;
        entry.propName = prop.name;
        entry.propPrettyName = prop.prettyName;
        mEntries.append(entry);

        mainLay->addWidget(new QLabel(entry.propPrettyName));

        QFormLayout *subLay = new QFormLayout;
        mainLay->addLayout(subLay);
    }
}

template<typename NodeTraits>
void NodeGroupEditWidget<NodeTraits>::reloadGroup(ViewManager *viewMgr)
{
    QVBoxLayout *mainLay = static_cast<QVBoxLayout *>(layout());

    QVector<ObjectProperty> objProps;
    NodeTraits::getObjectProperties(mItems.first(), objProps);

    for(int i = 0; i < mEntries.size(); i++)
    {
        PropertyEntry &entry = mEntries[i];

        // Label + Sublayout -> i * 2 + 1
        QFormLayout *subLay = static_cast<QFormLayout *>(mainLay->itemAt(i * 2 + 1));

        // Clear previous layout
        for(int row = entry.mValues.size() - 1; row >= 0; row--)
            subLay->removeRow(row);

        entry.mValues.clear();
    }

    for(Node *item : mItems)
    {
        QJsonObject settings;
        NodeTraits::saveToJSON(item, settings);

        for(int i = 0; i < mEntries.size(); i++)
        {
            PropertyEntry& entry = mEntries[i];
            const ObjectProperty& prop = objProps.at(i);

            const QString objName = settings.value(prop.name).toString();
            QString objType;

            const bool needsType = !prop.interface.isEmpty() || prop.types.size() != 1;
            if(needsType)
                objType = settings.value(prop.getTypeProp()).toString();
            else
                objType = prop.types.first();

            if(objName.isEmpty())
                objType.clear();

            bool found = false;
            for(PropertyValue &val : entry.mValues)
            {
                if(val.origObjectType == objType && val.origObjectName == objName)
                {
                    found = true;
                    val.items.append(item);
                    break;
                }
            }

            if(!found)
            {
                PropertyValue val;
                val.origObjectType = objType;
                val.origObjectName = objName;
                val.items.append(item);
                entry.mValues.append(val);
            }
        }
    }

    ModeManager *modeMgr = viewMgr->modeMgr();
    SimulationObjectFactory *factory = modeMgr->objectFactory();
    const QStringList objTypes = factory->getRegisteredTypes();

    for(int i = 0; i < mEntries.size(); i++)
    {
        PropertyEntry &entry = mEntries[i];

        std::sort(entry.mValues.begin(), entry.mValues.end(),
                  [objTypes](const PropertyValue& lhs, const PropertyValue& rhs) -> bool
        {
            if(lhs.origObjectType == rhs.origObjectType)
                return lhs.origObjectName < rhs.origObjectName;

            return objTypes.indexOf(lhs.origObjectType) < objTypes.indexOf(rhs.origObjectType);
        });

        const ObjectProperty& prop = objProps.at(i);

        QStringList types = prop.types;
        if(!prop.interface.isEmpty())
            types = factory->typesForInterface(prop.interface);

        // Label + Sublayout -> i * 2
        QFormLayout *subLay = static_cast<QFormLayout *>(mainLay->itemAt(i * 2 + 1));

        for(PropertyValue &val : entry.mValues)
        {
            val.mObjEdit = new SimulationObjectLineEdit(viewMgr, types);

            if(!val.origObjectName.isEmpty())
            {
                AbstractSimulationObject *obj = nullptr;

                auto model = modeMgr->modelForType(val.origObjectType);
                if(model)
                    obj = model->getObjectByName(val.origObjectName);

                val.mObjEdit->setObject(obj);
            }

            QString prettyName = factory->prettyName(val.origObjectType);
            QString entryName = QLatin1String("%1 <b>%2</b>").arg(prettyName, val.origObjectName);
            subLay->addRow(entryName, val.mObjEdit);
        }
    }
}

template<typename NodeTraits>
void NodeGroupEditWidget<NodeTraits>::saveChanges(ViewManager *viewMgr) const
{
    QVector<ObjectProperty> objProps;
    NodeTraits::getObjectProperties(mItems.first(), objProps);

    QHash<Node *, QJsonObject> mSettings;

    for(Node *item : mItems)
    {
        QJsonObject settings;
        NodeTraits::saveToJSON(item, settings);

        bool needsReload = false;

        for(int i = 0; i < mEntries.size(); i++)
        {
            const PropertyEntry& entry = mEntries.at(i);
            const ObjectProperty& prop = objProps.at(i);

            const QString objName = settings.value(prop.name).toString();
            QString objType;

            const bool needsType = !prop.interface.isEmpty() || prop.types.size() != 1;
            if(needsType)
                objType = settings.value(prop.getTypeProp()).toString();
            else
                objType = prop.types.first();

            if(objName.isEmpty())
                objType.clear();

            for(const PropertyValue &val : entry.mValues)
            {
                if(val.origObjectType == objType && val.origObjectName == objName)
                {
                    AbstractSimulationObject *newObj = val.mObjEdit->getObject();

                    bool changed = false;
                    if(newObj)
                    {
                        if(val.origObjectName.isEmpty())
                            changed = true;
                        else if(val.origObjectName != newObj->name() ||
                                val.origObjectType != newObj->getType())
                            changed = true;
                    }
                    else if(!val.origObjectName.isEmpty())
                        changed = true;

                    if(!changed)
                        break;

                    settings[prop.name] = newObj ? newObj->name() : QString();
                    if(needsType)
                        settings[prop.getTypeProp()] = newObj ? newObj->getType() : QString();

                    needsReload = true;
                    break;
                }
            }
        }

        if(needsReload)
            NodeTraits::loadFromJSON(item, settings, viewMgr->modeMgr());
    }
}

template<typename NodeTraits>
void NodeGroupEditWidget<NodeTraits>::replaceName(ViewManager *viewMgr,
                                                  const QString &oldStr, const QString &newStr)
{
    ModeManager *modeMgr = viewMgr->modeMgr();
    SimulationObjectFactory *factory = modeMgr->objectFactory();

    for(int i = 0; i < mEntries.size(); i++)
    {
        PropertyEntry &entry = mEntries[i];

        for(const PropertyValue &val : entry.mValues)
        {
            if(!val.origObjectName.contains(oldStr))
                continue;

            QString newName = val.origObjectName;
            newName.replace(oldStr, newStr);
            newName = newName.trimmed();

            if(val.origObjectName == newName)
                continue;

            AbstractSimulationObject *obj = nullptr;

            auto model = modeMgr->modelForType(val.origObjectType);
            if(!model)
                continue;

            obj = model->getObjectByName(newName);

            if(obj)
            {
                // Set new object
                val.mObjEdit->setObject(obj);
                continue;
            }

            // Clone existing object
            obj = val.mObjEdit->getObject();

            QJsonObject objSettings;
            obj->saveToJSON(objSettings);

            obj = factory->createItem(model);
            obj->setName(newName);
            model->addObject(obj);

            objSettings["name"] = newName;
            obj->loadFromJSON(objSettings, LoadPhase::Creation);
            obj->loadFromJSON(objSettings, LoadPhase::AllCreated);

            val.mObjEdit->setObject(obj);
        }
    }
}

#endif // ITEM_OBJECT_REPLACE_DLG_IMPL_HPP
