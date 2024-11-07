/**
 * src/objects/lever/model/genericlevermodel.h
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

#ifndef ACEI_LEVER_MODEL_H
#define ACEI_LEVER_MODEL_H

#include <QAbstractListModel>

#include <QVector>

class GenericLeverObject;

class ModeManager;

class QJsonObject;

class GenericLeverModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GenericLeverModel(ModeManager *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    void addLever(GenericLeverObject *r);
    void removeLever(GenericLeverObject *r);

    GenericLeverObject *leverAt(int row) const;

    GenericLeverObject *getLever(const QString& name);

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
    void onLeverChanged(GenericLeverObject *r);
    void onLeverStateChanged(GenericLeverObject *r);
    void onLeverDestroyed(QObject *obj);

private:
    void updateLeverRow(GenericLeverObject *r);
    void onLeverEdited();

private:
    QVector<GenericLeverObject *> mLevers;

    ModeManager *mModeMgr;

    bool mHasUnsavedChanges = false;
};

#endif // ACEI_LEVER_MODEL_H
