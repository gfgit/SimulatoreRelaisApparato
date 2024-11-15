/**
 * src/objects/abstractsimulationobject.h
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

#ifndef ABSTRACTSIMULATIONOBJECT_H
#define ABSTRACTSIMULATIONOBJECT_H

#include <QObject>
#include <QVector>

class AbstractSimulationObjectModel;

class AbstractCircuitNode;
class QJsonObject;

class AbstractSimulationObject : public QObject
{
    Q_OBJECT
public:
    explicit AbstractSimulationObject(AbstractSimulationObjectModel *m);

    virtual QString getType() const = 0;

    virtual QObject *getInterface(const QString& ifaceName);

    virtual bool loadFromJSON(const QJsonObject& obj);
    virtual void saveToJSON(QJsonObject& obj) const;

    QString name() const;
    bool setName(const QString &newName);

    QString description() const;
    void setDescription(const QString &newDescription);

    inline QString uniqueId() const
    {
        return getType() + QLatin1Char('.') + name();
    }

    inline AbstractSimulationObjectModel *model() const
    {
        return mModel;
    }

    // Nodes in which this object is referenced
    virtual QVector<AbstractCircuitNode *> nodes() const;

signals:
    void nameChanged(const QString& name);
    void descriptionChanged(const QString& name);

    void settingsChanged(AbstractSimulationObject *self);
    void stateChanged(AbstractSimulationObject *self);

    void nodesChanged();

private:
    AbstractSimulationObjectModel *mModel;

    QString mName;
    QString mDescription;
};

#endif // ABSTRACTSIMULATIONOBJECT_H
