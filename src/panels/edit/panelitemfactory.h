/**
 * src/panels/edit/panelitemfactory.h
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

#ifndef PANEL_ITEM_FACTORY_H
#define PANEL_ITEM_FACTORY_H

#include <QObject>
#include <QString>
#include <QVector>

#include "../../utils/tilerotate.h"

class AbstractPanelItem;
class PanelScene;

class ModeManager;
class ViewManager;

class QWidget;

class PanelItemFactory : public QObject
{
    Q_OBJECT
public:
    PanelItemFactory(QObject *parent);

    typedef AbstractPanelItem *(*CreateFunc)(PanelScene *parent, ModeManager *mgr);
    typedef QWidget*(*EditFunc)(AbstractPanelItem *item, ViewManager *mgr);

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

        NeedsName needsName = NeedsName::OnlyOnEditing;
    };

    QStringList getRegisteredTypes() const;

    AbstractPanelItem *createItem(const QString& nodeType,
                                  PanelScene *scene);
    void editItem(QWidget *parent,
                  AbstractPanelItem *item,
                  ViewManager *viewMgr,
                  bool allowDelete = true);

    QString prettyName(const QString& nodeType) const;
    NeedsName needsName(const QString &nodeType) const;

    void registerFactory(const FactoryItem& factory);

private:
    const FactoryItem *getItemForType(const QString& nodeType) const;

private:
    QVector<FactoryItem> mItems;
};

#endif // PANEL_ITEM_FACTORY_H
