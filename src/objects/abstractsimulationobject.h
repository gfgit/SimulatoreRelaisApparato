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
#include <QHash>

#include "../enums/loadphase.h"

class AbstractSimulationObjectModel;

class AbstractObjectInterface;

class AbstractCircuitNode;
class QJsonObject;

class QCborMap;

class AbstractSimulationObject : public QObject
{
    Q_OBJECT
public:
    explicit AbstractSimulationObject(AbstractSimulationObjectModel *m);
    ~AbstractSimulationObject();

    virtual QString getType() const = 0;

    AbstractObjectInterface *getAbstractInterface(const QString& ifaceType) const;

    template <typename Iface>
    inline Iface *getInterface() const
    {
        return static_cast<Iface *>(getAbstractInterface(Iface::IfaceType));
    }

    const QVector<AbstractObjectInterface *>& getAllInterfaces() const
    {
        return mInterfaces;
    }

    virtual bool loadFromJSON(const QJsonObject& obj, LoadPhase phase);
    virtual void saveToJSON(QJsonObject& obj) const;

    virtual void getReferencedObjects(QSet<AbstractSimulationObject *> &result);

    void setReplicaMode(bool on);
    virtual bool setReplicaState(const QCborMap& replicaState);
    virtual void getReplicaState(QCborMap& replicaState) const;

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
    // If result is nullptr, it just returns number of referencing getReferencingNodes
    virtual int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const;

    inline bool isRemoteReplica() const
    {
        return mIsRemoteReplica;
    }

signals:
    void nameChanged(AbstractSimulationObject *self,
                     const QString& name, const QString& oldName);
    void descriptionChanged(const QString& name);

    void settingsChanged(AbstractSimulationObject *self);
    void stateChanged(AbstractSimulationObject *self);
    void nodesChanged(AbstractSimulationObject *self);

    void interfacePropertyChanged(const QString& ifaceName,
                                  const QString& propName,
                                  const QVariant& value);


private slots:
    void onTrackedObjectDestroyed_slot(QObject *obj);

protected:
    void timerEvent(QTimerEvent *e) override;

    void addInterface(AbstractObjectInterface *iface);
    void removeInterface(AbstractObjectInterface *iface);

    friend class AbstractObjectInterface;
    virtual void onInterfaceChanged(AbstractObjectInterface *iface,
                                    const QString &propName,
                                    const QVariant &value);

    void trackObject(AbstractSimulationObject *obj);
    void untrackObject(AbstractSimulationObject *obj);

    virtual void onTrackedObjectDestroyed(AbstractSimulationObject *obj);

    virtual void onReplicaModeChanged(bool on);

private:
    AbstractSimulationObjectModel *mModel;

    QString mName;
    QString mDescription;

    QVector<AbstractObjectInterface *> mInterfaces;

    QHash<AbstractSimulationObject *, int> mTrackedObjects;

    bool mIsRemoteReplica = false;
};

#endif // ABSTRACTSIMULATIONOBJECT_H
