#include "circuitnodeobjectreplacedlg.h"

#include "../graphs/abstractnodegraphitem.h"
#include "../nodes/abstractcircuitnode.h"

#include <QJsonObject>

#include "../../objects/simulationobjectlineedit.h"
#include "../../objects/simulationobjectfactory.h"
#include "../../objects/abstractsimulationobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../edit/nodeeditfactory.h"

#include "../../views/modemanager.h"
#include "../../views/viewmanager.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include <QPushButton>
#include <QLineEdit>

#include <QDialogButtonBox>

CircuitNodeObjectReplaceDlg::CircuitNodeObjectReplaceDlg(ViewManager *viewMgr,
                                                         const QVector<AbstractNodeGraphItem *> &items,
                                                         QWidget *parent)
    : QDialog{parent}
    , mViewMgr(viewMgr)
    , mItems(items)
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);

    QPushButton *replaceStrBut = new QPushButton(tr("Replace Name String"));
    mainLay->addWidget(replaceStrBut);

    mGroupsLay = new QVBoxLayout;
    mainLay->addLayout(mGroupsLay);

    createGroups();
    reloadGroups();

    connect(replaceStrBut, &QPushButton::clicked,
            this, &CircuitNodeObjectReplaceDlg::onReplaceName);
}

void CircuitNodeObjectReplaceDlg::replaceName(const QString &oldStr, const QString &newStr)
{
    for(NodeGroupEditWidget *w : mGroups)
    {
        w->replaceName(mViewMgr, oldStr, newStr);
    }

    saveChanges();
    reloadGroups();
}

void CircuitNodeObjectReplaceDlg::done(int result)
{
    if(result == QDialog::Accepted)
        saveChanges();

    QDialog::done(result);
}

void CircuitNodeObjectReplaceDlg::reloadGroups()
{
    for(NodeGroupEditWidget *w : mGroups)
    {
        w->reloadGroup(mViewMgr);
    }
}

void CircuitNodeObjectReplaceDlg::createGroups()
{
    QHash<QString, QVector<AbstractNodeGraphItem *>> groups;
    QStringList ignoreTypes;

    for(AbstractNodeGraphItem *item : mItems)
    {
        const QString nodeType = item->getAbstractNode()->nodeType();
        if(ignoreTypes.contains(nodeType))
            continue;

        auto it = groups.find(nodeType);
        if(it != groups.end())
        {
            it.value().append(item);
            continue;
        }

        QVector<AbstractCircuitNode::ObjectProperty> objProps;
        item->getAbstractNode()->getObjectProperties(objProps);
        if(objProps.isEmpty())
        {
            ignoreTypes.append(nodeType);
            continue;
        }

        groups.insert(nodeType, {item});
    }

    ModeManager *modeMgr = mViewMgr->modeMgr();
    NodeEditFactory *factory = modeMgr->circuitFactory();
    const QStringList nodeTypes = factory->getRegisteredTypes();

    for(const QString& nodeType : nodeTypes)
    {
        auto it = groups.find(nodeType);
        if(it == groups.end())
            continue;

        NodeGroupEditWidget *w = new NodeGroupEditWidget(it.value());
        w->createGroup(mViewMgr);

        w->setTitle(factory->prettyName(nodeType));
        mGroups.append(w);
        mGroupsLay->addWidget(w);
    }
}

void CircuitNodeObjectReplaceDlg::saveChanges()
{
    for(NodeGroupEditWidget *w : mGroups)
    {
        w->saveChanges();
    }
}

void CircuitNodeObjectReplaceDlg::onReplaceName()
{
    QPointer<QDialog> dlg = new QDialog(this);

    QVBoxLayout *lay = new QVBoxLayout(dlg);

    lay->addWidget(new QLabel(tr("Replace a string in object names.\n"
                                 "If an object with new name exists it replaces the old one.\n"
                                 "If not, it's created by cloning the old one.")));

    QLineEdit *oldEdit = new QLineEdit;
    oldEdit->setPlaceholderText(tr("Old string"));
    lay->addWidget(oldEdit);

    QLineEdit *newEdit = new QLineEdit;
    newEdit->setPlaceholderText(tr("New string"));
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

void CircuitNodeObjectReplaceDlg::batchNodeEdit(const QVector<AbstractNodeGraphItem *>& items,
                                                ViewManager *viewMgr,
                                                QWidget *parent)
{
    AbstractNodeGraphItem *curItem = items.first();

    QJsonObject origSettings;
    curItem->saveToJSON(origSettings);

    // Edit first item, do not allow delete
    auto editFactory = viewMgr->modeMgr()->circuitFactory();
    editFactory->editItem(parent, curItem, viewMgr, false);

    QJsonObject newSettings;
    curItem->saveToJSON(newSettings);

    const QStringList ignoreKeys{"x", "y", "name", "rotation",
                                 "text_rotation", "type"};
    QStringList modifiedKeys;

    for(const QString& key : newSettings.keys())
    {
        if(ignoreKeys.contains(key))
            continue;

        const QJsonValueRef oldVal = origSettings[key];
        const QJsonValueRef newVal = newSettings[key];

        if(oldVal.isObject() || newVal.isObject())
            continue; // Skip sub objects

        if(oldVal.toVariant() != newVal.toVariant())
            modifiedKeys.append(key);
    }

    for(AbstractNodeGraphItem *item : std::as_const(items))
    {
        if(item == curItem)
            continue;

        QJsonObject settings;
        item->saveToJSON(settings);

        for(const QString& key : std::as_const(modifiedKeys))
        {
            // Set new value
            settings[key] = newSettings[key];
        }

        item->loadFromJSON(settings);
    }
}

NodeGroupEditWidget::NodeGroupEditWidget(const QVector<AbstractNodeGraphItem *> &items)
    : QGroupBox()
    , mItems(items)
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);
}

void NodeGroupEditWidget::createGroup(ViewManager *viewMgr)
{
    QVBoxLayout *mainLay = static_cast<QVBoxLayout *>(layout());

    QVector<AbstractCircuitNode::ObjectProperty> objProps;
    mItems.first()->getAbstractNode()->getObjectProperties(objProps);

    for(const AbstractCircuitNode::ObjectProperty& prop : std::as_const(objProps))
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

void NodeGroupEditWidget::reloadGroup(ViewManager *viewMgr)
{
    QVBoxLayout *mainLay = static_cast<QVBoxLayout *>(layout());

    QVector<AbstractCircuitNode::ObjectProperty> objProps;
    mItems.first()->getAbstractNode()->getObjectProperties(objProps);

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

    for(AbstractNodeGraphItem *item : mItems)
    {
        QJsonObject settings;
        item->saveToJSON(settings);

        for(int i = 0; i < mEntries.size(); i++)
        {
            PropertyEntry& entry = mEntries[i];
            const AbstractCircuitNode::ObjectProperty& prop = objProps.at(i);

            const QString objName = settings.value(prop.name).toString();
            QString objType;

            const bool needsType = !prop.interface.isEmpty() || prop.types.size() != 1;
            if(needsType)
                objType = settings.value(prop.name + QLatin1String("_type")).toString();
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

        const AbstractCircuitNode::ObjectProperty& prop = objProps.at(i);

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

void NodeGroupEditWidget::saveChanges() const
{
    QVector<AbstractCircuitNode::ObjectProperty> objProps;
    mItems.first()->getAbstractNode()->getObjectProperties(objProps);

    QHash<AbstractNodeGraphItem *, QJsonObject> mSettings;

    for(AbstractNodeGraphItem *item : mItems)
    {
        QJsonObject settings;
        item->saveToJSON(settings);

        bool needsReload = false;

        for(int i = 0; i < mEntries.size(); i++)
        {
            const PropertyEntry& entry = mEntries.at(i);
            const AbstractCircuitNode::ObjectProperty& prop = objProps.at(i);

            const QString objName = settings.value(prop.name).toString();
            QString objType;

            const bool needsType = !prop.interface.isEmpty() || prop.types.size() != 1;
            if(needsType)
                objType = settings.value(prop.name + QLatin1String("_type")).toString();
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
                        settings[prop.name + QLatin1String("_type")] = newObj ? newObj->getType() : QString();

                    needsReload = true;
                    break;
                }
            }
        }

        if(needsReload)
            item->loadFromJSON(settings);
    }
}

void NodeGroupEditWidget::replaceName(ViewManager *viewMgr,
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
