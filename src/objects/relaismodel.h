/**
 * src/objects/relaismodel.h
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

#ifndef RELAISMODEL_H
#define RELAISMODEL_H

#include <QAbstractListModel>

#include <QVector>

class AbstractRelais;

class ModeManager;

class QJsonObject;

class RelaisModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RelaisModel(ModeManager *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    void addRelay(AbstractRelais *r);
    void removeRelay(AbstractRelais *r);

    AbstractRelais *relayAt(int row) const;

    AbstractRelais *getRelay(const QString& name);

    void clear();

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    bool loadFromJSON(const QJsonObject& obj);
    void saveToJSON(QJsonObject& obj) const;

    bool isNameAvailable(const QString& name) const;

    inline bool hasUnsavedChanges() const
    {
        return mHasUnsavedChanges;
    }

    void resetHasUnsavedChanges();

signals:
    void modelEdited(bool val);

private slots:
    void onRelayChanged(AbstractRelais *r);
    void onRelayStateChanged(AbstractRelais *r);
    void onRelayDestroyed(QObject *obj);

private:
    void updateRelayRow(AbstractRelais *r);
    void onRelayEdited();

private:
    QVector<AbstractRelais *> mRelais;

    ModeManager *mModeMgr;

    bool mHasUnsavedChanges = false;
};

#endif // RELAISMODEL_H
