/**
 * src/objects/abstractsimulationobjectmodel.h
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

#ifndef ABSTRACTSIMULATIONOBJECTMODEL_H
#define ABSTRACTSIMULATIONOBJECTMODEL_H

#include <QAbstractTableModel>

#include <QVector>

class AbstractSimulationObject;

class ModeManager;

class QJsonObject;

class AbstractSimulationObjectModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    AbstractSimulationObjectModel(ModeManager *mgr,
                                  const QString& objTypeName,
                                  QObject *parent = nullptr);
    ~AbstractSimulationObjectModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    AbstractSimulationObject *objectAt(int row) const;

    AbstractSimulationObject *getObjectByName(const QString& name) const;

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    inline QString getObjectType() const
    {
        return mObjectType;
    }

    QString getObjectPrettyName() const;

    void clear();
    bool loadFromJSON(const QJsonObject& modelObj);
    void saveToJSON(QJsonObject& modelObj) const;

    bool isNameAvailable(const QString& name) const;

    inline bool hasUnsavedChanges() const
    {
        return mHasUnsavedChanges;
    }

    void resetHasUnsavedChanges();

    void addObject(AbstractSimulationObject *item);
    void removeObject(AbstractSimulationObject *item);

signals:
    void modelEdited(bool val);

private slots:
    void onObjectChanged(AbstractSimulationObject *item);
    void onObjectStateChanged(AbstractSimulationObject *item);
    void onObjectDestroyed(QObject *obj);

private:
    void updateObjectRow(AbstractSimulationObject *item);
    void setModelEdited();
    void clearInternal();

protected:
    virtual void addObjectInternal(AbstractSimulationObject *item);
    virtual void removeObjectInternal(AbstractSimulationObject *item);

private:
    ModeManager *mModeMgr;

    QVector<AbstractSimulationObject *> mObjects;

    const QString mObjectType;

    bool mHasUnsavedChanges = false;
};

#endif // ABSTRACTSIMULATIONOBJECTMODEL_H
