/**
 * src/views/uilayoutsmodel.h
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

#ifndef UILAYOUTSMODEL_H
#define UILAYOUTSMODEL_H

#include <QAbstractTableModel>

#include <QVector>

class ViewManager;

class UILayoutsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        Name = 0,
        NCols
    };

    explicit UILayoutsModel(ViewManager *viewMgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    inline QString layoutNameAt(int row) const
    {
        if(row < 0 || row >= mLayouts.size())
            return QString();

        return mLayouts.at(row).name;
    }

    void clear();
    void loadFromLayoutFile();

    static QString getLayoutFilePath(const QString& mainFileName);

    bool loadLayout(const QString& layoutName);
    bool saveLayout(const QString& layoutName) const;

    bool renameLayout(const QString& oldName, const QString& newName);

    QString layoutToLoadAtStart() const;
    void setLayoutToLoadAtStart(const QString &newLayoutToLoadAtStart);

    inline bool layoutExists(const QString& name) const
    {
        for(const LayoutEntry& entry : mLayouts)
        {
            if(entry.name == name)
                return true;
        }

        return false;
    }

    bool addLayout(const QString& name);
    void removeLayoutAt(int row);

signals:
    void layoutToLoadAtStartChanged();

private slots:
    void onFileChanged(const QString& newFile, const QString& oldFile);

private:
    QJsonObject getLayoutFileRoot(bool &ok) const;
    void saveLayoutFileRoot(const QJsonObject& rootObj) const;

private:
    ViewManager *mViewMgr = nullptr;

    struct LayoutEntry
    {
        QString name;
    };

    QVector<LayoutEntry> mLayouts;
    QString mLayoutToLoadAtStart;
};

#endif // UILAYOUTSMODEL_H
