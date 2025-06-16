/**
 * src/views/uilayoutsmodel.cpp
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

#include "uilayoutsmodel.h"

#include "viewmanager.h"
#include "modemanager.h"
#include "../network/remotemanager.h"

#include "layoutloader.h"

#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFont>

static constexpr const QLatin1String LayoutFileExtension(".simrelayout");

static constexpr const QLatin1String StartLayoutKey("start_layout");
static constexpr const QLatin1String LayoutArrayKey("layouts");
static constexpr const QLatin1String LayoutNameKey("name");
static constexpr const QLatin1String LayoutDataKey("data");
static constexpr const QLatin1String LayoutConfigKey("config");

static constexpr const QLatin1String OnlineByDefaultKey("default_online");

UILayoutsModel::UILayoutsModel(ViewManager *viewMgr, QObject *parent)
    : QAbstractTableModel(parent)
    , mViewMgr(viewMgr)
{
    connect(mViewMgr->modeMgr(), &ModeManager::fileChanged,
            this, &UILayoutsModel::onFileChanged);
}

QVariant UILayoutsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case Name:
            return tr("Name");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int UILayoutsModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mLayouts.size();
}

int UILayoutsModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant UILayoutsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mLayouts.size())
        return QVariant();

    const LayoutEntry& item = mLayouts.at(idx.row());

    switch (idx.column())
    {
    case Name:
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            return item.name;
        }
        case Qt::ToolTipRole:
        {
            if(mLayoutToLoadAtStart == item.name)
                return tr("%1\n"
                          "This layout will be loaded on start.")
                        .arg(item.name);
            else
                return item.name;
        }
        case Qt::FontRole:
        {
            if(mLayoutToLoadAtStart == item.name)
            {
                // Bold
                QFont f;
                f.setBold(true);
                return f;
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
    return QVariant();
}

void UILayoutsModel::clear()
{
    beginResetModel();

    mLayouts.clear();
    mLayoutToLoadAtStart = QString();

    endResetModel();

    emit layoutToLoadAtStartChanged();
}

void UILayoutsModel::loadFromLayoutFile()
{
    beginResetModel();

    mLayouts.clear();
    mLayoutToLoadAtStart = QString();

    bool ok = false;
    const QJsonObject rootObj = getLayoutFileRoot(ok);
    if(!ok)
    {
        endResetModel();
        return;
    }

    RemoteManager *remoteMgr = mViewMgr->modeMgr()->getRemoteManager();
    remoteMgr->setOnlineByDefault(rootObj.value(OnlineByDefaultKey).toBool(false));
    if(remoteMgr->onlineByDefault())
        remoteMgr->setOnline(true);

    setLayoutToLoadAtStart(rootObj.value(StartLayoutKey).toString().trimmed());

    const QJsonArray layoutArr = rootObj.value(LayoutArrayKey).toArray();

    QSet<QString> uniqueNames;

    for(const QJsonValue& v : layoutArr)
    {
        const QJsonObject obj = v.toObject();

        LayoutEntry item;
        item.name = obj.value(LayoutNameKey).toString().trimmed();
        if(item.name.isEmpty() || uniqueNames.contains(item.name))
            continue;

        uniqueNames.insert(item.name);

        mLayouts.append(item);
    }

    std::sort(mLayouts.begin(), mLayouts.end(),
              [](const LayoutEntry& lhs, const LayoutEntry& rhs)
    {
        return lhs.name < rhs.name;
    });

    endResetModel();

    emit layoutToLoadAtStartChanged();
}

QString UILayoutsModel::getLayoutFilePath(const QString &mainFileName)
{
    const int idx = mainFileName.lastIndexOf('.');
    if(idx < 0)
        return QString();

    QString layoutFileName = mainFileName.left(idx);
    layoutFileName.append(LayoutFileExtension);
    return layoutFileName;
}

bool UILayoutsModel::loadLayout(const QString &layoutName)
{
    bool ok = false;
    const QJsonObject rootObj = getLayoutFileRoot(ok);
    if(!ok)
    {
        return false;
    }

    const QJsonArray layoutArr = rootObj.value(LayoutArrayKey).toArray();

    for(const QJsonValue& v : layoutArr)
    {
        const QJsonObject obj = v.toObject();

        const QString name = obj.value(LayoutNameKey).toString().trimmed();
        if(name != layoutName)
            continue;

        const QString data = obj.value(LayoutDataKey).toString();
        if(data.isEmpty())
            return false;

        LayoutLoader::loadLayout(data.toUtf8());
        LayoutLoader::loadLayoutConfig(obj.value(LayoutConfigKey).toObject());
        return true;
    }

    return false;
}

bool UILayoutsModel::saveLayout(const QString &layoutName) const
{
    bool ok = false;
    QJsonObject rootObj = getLayoutFileRoot(ok);

    QJsonArray layoutArr = rootObj.value(LayoutArrayKey).toArray();

    for(auto it = layoutArr.begin(); it != layoutArr.end();)
    {
        const QJsonObject obj = it->toObject();

        const QString name = obj.value(LayoutNameKey).toString().trimmed();
        if(name == layoutName || !layoutExists(name))
        {
            it = layoutArr.erase(it);
            continue;
        }

        it++;
    }

    QJsonObject obj;
    obj[LayoutNameKey] = layoutName;
    obj[LayoutDataKey] = QString::fromUtf8(LayoutLoader::saveLayout());
    obj[LayoutConfigKey] = LayoutLoader::saveLayoutConfig();
    layoutArr.append(obj);

    rootObj[LayoutArrayKey] = layoutArr;
    rootObj[StartLayoutKey] = mLayoutToLoadAtStart;
    rootObj[OnlineByDefaultKey] = mViewMgr->modeMgr()->getRemoteManager()->onlineByDefault();

    saveLayoutFileRoot(rootObj);
    return true;
}

bool UILayoutsModel::renameLayout(const QString &oldName, const QString &newName)
{
    if(layoutExists(newName))
        return false;

    int row = 0;
    for(auto it = mLayouts.begin(); it != mLayouts.end(); it++)
    {
        if(it->name == oldName)
        {
            it->name = newName;
            break;
        }

        row++;
    }

    bool ok = false;
    QJsonObject rootObj = getLayoutFileRoot(ok);

    QJsonArray layoutArr = rootObj.value(LayoutArrayKey).toArray();

    QJsonObject origLayout;

    for(auto it = layoutArr.begin(); it != layoutArr.end();)
    {
        const QJsonObject obj = it->toObject();

        const QString name = obj.value(LayoutNameKey).toString().trimmed();
        if(name == oldName)
            origLayout = obj;

        if(name == oldName || name == newName || !layoutExists(name))
        {
            it = layoutArr.erase(it);
            continue;
        }

        it++;
    }

    // Rename
    origLayout[LayoutNameKey] = newName;
    layoutArr.append(origLayout);

    rootObj[LayoutArrayKey] = layoutArr;
    rootObj[StartLayoutKey] = mLayoutToLoadAtStart;

    saveLayoutFileRoot(rootObj);

    emit dataChanged(index(row, 0),
                     index(row, NCols - 1));

    return true;
}

QJsonObject UILayoutsModel::getLayoutFileRoot(bool &ok) const
{
    ok = false;

    const QString layoutFileName = getLayoutFilePath(mViewMgr->modeMgr()->filePath());
    if(layoutFileName.isEmpty())
        return {};

    QFile layoutFile(layoutFileName);
    if(!layoutFile.exists())
    {
        // Error
        qWarning() << "Layout file does not exist:" << layoutFileName;
        return {};
    }

    if(!layoutFile.open(QFile::ReadOnly))
    {
        // Error
        qWarning() << "Layout file open error:" << layoutFile.errorString();
        return {};
    }

    const QJsonDocument doc = QJsonDocument::fromJson(layoutFile.readAll());

    const QJsonObject rootObj = doc.object();

    ok = true;
    return rootObj;
}

void UILayoutsModel::saveLayoutFileRoot(const QJsonObject &rootObj) const
{
    const QString layoutFileName = getLayoutFilePath(mViewMgr->modeMgr()->filePath());
    if(layoutFileName.isEmpty())
        return;

    QFile layoutFile(layoutFileName);
    if(!layoutFile.open(QFile::WriteOnly))
    {
        // Error
        qWarning() << "Layout file save error:" << layoutFile.errorString();
        return;
    }

    QJsonDocument doc(rootObj);
    layoutFile.write(doc.toJson());
}

QString UILayoutsModel::layoutToLoadAtStart() const
{
    return mLayoutToLoadAtStart;
}

void UILayoutsModel::setLayoutToLoadAtStart(const QString &newLayoutToLoadAtStart)
{
    if(mLayoutToLoadAtStart == newLayoutToLoadAtStart)
        return;

    mLayoutToLoadAtStart = newLayoutToLoadAtStart;

    emit layoutToLoadAtStartChanged();

    emit dataChanged(index(0, 0),
                     index(rowCount() - 1, columnCount() - 1));
}

bool UILayoutsModel::addLayout(const QString &name)
{
    if(mViewMgr->modeMgr()->filePath().isEmpty())
        return false; // Needs file save first

    const QString nameTrimmed = name.trimmed();

    if(nameTrimmed.isEmpty() || layoutExists(nameTrimmed))
        return false;

    const int row = mLayouts.size();
    beginInsertRows(QModelIndex(), row, row);

    mLayouts.append({nameTrimmed});

    endInsertRows();

    return true;
}

void UILayoutsModel::removeLayoutAt(int row)
{
    if(row < 0 || row >= mLayouts.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);

    LayoutEntry item = mLayouts.takeAt(row);

    if(item.name == layoutToLoadAtStart())
        setLayoutToLoadAtStart(QString());

    endRemoveRows();
}

void UILayoutsModel::onFileChanged(const QString &newFile, const QString &oldFile)
{
    if(oldFile.isEmpty() || oldFile == newFile)
        return;

    // Copy layout file to new name
    const QString oldLayoutFile = getLayoutFilePath(oldFile);
    const QString newLayoutFile = getLayoutFilePath(newFile);

    if(oldLayoutFile.isEmpty() || newLayoutFile.isEmpty())
        return;

    QFile::copy(oldLayoutFile, newLayoutFile);
}
