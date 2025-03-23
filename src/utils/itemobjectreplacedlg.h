/**
 * src/utils/itemobjectreplacedlg.h
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

#ifndef ITEM_OBJECT_REPLACE_DLG_H
#define ITEM_OBJECT_REPLACE_DLG_H

#include <QDialog>
#include <QGroupBox>
#include <QVector>

class SimulationObjectLineEdit;

class ViewManager;

class QVBoxLayout;

template <typename NodeTraits>
class NodeGroupEditWidget;

template <typename NodeTraits>
class ItemObjectReplaceDlg : public QDialog
{
public:
    typedef typename NodeTraits::Node Node;
    typedef NodeGroupEditWidget<NodeTraits> NodeGroupEditWidget_;

    explicit ItemObjectReplaceDlg(ViewManager *viewMgr,
                                  const QVector<Node *>& items,
                                  QWidget *parent = nullptr);

    void replaceName(const QString& oldStr, const QString &newStr);

    static void batchNodeEdit(const QVector<Node *> &items,
                              ViewManager *viewMgr,
                              QWidget *parent = nullptr);
protected:
    void done(int result) override;

private:
    void reloadGroups();
    void createGroups();

    void saveChanges();

    void onReplaceName();

private:
    ViewManager *mViewMgr;
    QVBoxLayout *mGroupsLay;
    QVector<NodeGroupEditWidget_ *> mGroups;
    QVector<Node *> mItems;
};

template <typename NodeTraits>
class NodeGroupEditWidget : public QGroupBox
{
public:
    typedef typename NodeTraits::Node Node;
    explicit NodeGroupEditWidget(
            const QVector<Node *>& items);

public:
    void createGroup(ViewManager *viewMgr);
    void reloadGroup(ViewManager *viewMgr);

    void saveChanges(ViewManager *viewMgr) const;

    void replaceName(ViewManager *viewMgr,
                     const QString& oldStr, const QString &newStr);

private:
    QString mTypeName;

    struct PropertyValue
    {
        SimulationObjectLineEdit *mObjEdit;
        QVector<Node *> items;
        QString origObjectType;
        QString origObjectName;
    };

    struct PropertyEntry
    {
        QString propName;
        QString propPrettyName;
        QVector<PropertyValue> mValues;
    };

    QVector<PropertyEntry> mEntries;
    QVector<Node *> mItems;
};

#endif // ITEM_OBJECT_REPLACE_DLG_H
