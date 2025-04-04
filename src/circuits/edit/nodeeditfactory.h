/**
 * src/circuits/edit/nodeeditfactory.h
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

#ifndef NODEEDITFACTORY_H
#define NODEEDITFACTORY_H

#include <QObject>
#include <QString>
#include <QVector>

#include "../../utils/tilerotate.h"

class CableGraphItem;
class AbstractNodeGraphItem;
class AbstractCircuitNode;
class CircuitScene;

class ModeManager;
class ViewManager;

class QWidget;

class NodeEditFactory : public QObject
{
    Q_OBJECT
public:
    NodeEditFactory(QObject *parent);

    typedef AbstractNodeGraphItem *(*CreateFunc)(CircuitScene *parent, ModeManager *mgr);
    typedef QWidget*(*EditFunc)(AbstractNodeGraphItem *item, ViewManager *mgr);

    enum class NeedsName
    {
        Never = 0,
        Always = 1,
        OnlyOnEditing = 2
    };

    struct FactoryItem
    {
        QString nodeType;
        QString prettyName;
        CreateFunc create = nullptr;
        EditFunc edit = nullptr;
        QChar shortcutLetter;

        NeedsName needsName = NeedsName::OnlyOnEditing;
    };

    QStringList getRegisteredTypes() const;

    AbstractNodeGraphItem *createItem(const QString& nodeType,
                                      CircuitScene *scene);
    void editItem(QWidget *parent, AbstractNodeGraphItem *item,
                  ViewManager *viewMgr, bool allowDelete = true);
    void editCable(QWidget *parent, CableGraphItem *item);

    QString prettyName(const QString& nodeType) const;
    NeedsName needsName(const QString &nodeType) const;
    QChar letterForType(const QString& nodeType) const;

    QString typeForShortcutLetter(QChar letter) const;

    void registerFactory(const FactoryItem& factory);

private:
    const FactoryItem *getItemForType(const QString& nodeType) const;

private:
    QVector<FactoryItem> mItems;
};

#endif // NODEEDITFACTORY_H
