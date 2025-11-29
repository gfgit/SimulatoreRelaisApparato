/**
 * src/objects/traintastic/edit/signalindicatorlistview.cpp
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

#include "signalindicatorlistview.h"

#include "signalindicatorlistmodel.h"

#include "../../../objects/simulationobjectlineedit.h"
#include "../../../objects/simple_activable/lightbulbobject.h"
#include "../../../objects/traintastic/traintasticsignalobject.h"

#include "../../../views/viewmanager.h"

#include <QLineEdit>
#include <QTableView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPointer>

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

class SignalIndicatorEntryDialog : public QDialog
{
public:
    SignalIndicatorEntryDialog(QWidget *parent) : QDialog(parent)
    {
        QFormLayout *lay = new QFormLayout(this);

        lightEdit = new SimulationObjectLineEdit(ViewManager::self(),
                                                 {LightBulbObject::Type});
        lay->addRow(tr("Light:"), lightEdit);

        letterEdit = new QLineEdit;
        letterEdit->setMaxLength(1);
        lay->addRow(tr("Letter:"), letterEdit);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                     Qt::Horizontal);
        connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
        lay->addRow(box);
    }

    void setEntry(LightBulbObject *obj, const char letter)
    {
        lightEdit->setObject(obj);
        if(letter == ' ')
            letterEdit->setText(QString());
        else
            letterEdit->setText(QString::fromLatin1(&letter, 1));
    }

    char getLetter() const
    {
        if(letterEdit->text().isEmpty())
            return ' ';
        return letterEdit->text().at(0).toLatin1();
    }

    LightBulbObject *getLight() const
    {
        return static_cast<LightBulbObject *>(lightEdit->getObject());
    }

private:
    SimulationObjectLineEdit *lightEdit;
    QLineEdit *letterEdit;
};

SignalIndicatorListView::SignalIndicatorListView(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    mModel = new SignalIndicatorListModel(this);
    connect(mModel, &SignalIndicatorListModel::needsSave,
            this, &SignalIndicatorListView::needsSave);

    mView = new QTableView;
    mView->setSelectionBehavior(QTableView::SelectRows);
    mView->setEditTriggers(QTableView::NoEditTriggers);
    mView->setModel(mModel);
    lay->addWidget(mView);

    QVBoxLayout *butLay = new QVBoxLayout;
    lay->addLayout(butLay);

    mAddLightBut = new QPushButton(tr("Add Light"));
    mRemLightBut = new QPushButton(tr("Remove Light"));
    mEditLightBut = new QPushButton(tr("Edit Light"));
    mUpLightBut = new QPushButton(tr("Move Up"));
    mDownLightBut = new QPushButton(tr("Move Down"));

    butLay->addWidget(mAddLightBut);
    butLay->addWidget(mRemLightBut);
    butLay->addWidget(mEditLightBut);
    butLay->addWidget(mUpLightBut);
    butLay->addWidget(mDownLightBut);

    connect(mAddLightBut, &QPushButton::clicked,
            this, &SignalIndicatorListView::onAdd);
    connect(mRemLightBut, &QPushButton::clicked,
            this, &SignalIndicatorListView::onRemove);
    connect(mEditLightBut, &QPushButton::clicked,
            this, &SignalIndicatorListView::onEdit);
    connect(mUpLightBut, &QPushButton::clicked,
            this, [this]()
    {
        onMove(true);
    });
    connect(mDownLightBut, &QPushButton::clicked,
            this, [this]()
    {
        onMove(false);
    });
    connect(mView, &QTableView::activated,
            this, &SignalIndicatorListView::editIndex);
}

void SignalIndicatorListView::loadFrom(TraintasticSignalObject *item)
{
    const auto dirLights = item->directionLights();
    QVector<SignalIndicatorListModel::LightEntry> entries;
    entries.reserve(dirLights.size());
    for(const auto& entry : dirLights)
        entries.append({entry.light, entry.letter});
    mModel->setItems(entries);
}

void SignalIndicatorListView::saveTo(TraintasticSignalObject *item)
{
    if(!mModel->isChanged())
        return;

    QVector<TraintasticSignalObject::DirectionEntry> entries;
    entries.reserve(mModel->items().size());
    for(const auto& entry : mModel->items())
        entries.append({entry.light, entry.letter});
    item->setDirectionLights(entries);

    mModel->resetChanged();
}

void SignalIndicatorListView::onAdd()
{
    QPointer<SignalIndicatorEntryDialog> dlg = new SignalIndicatorEntryDialog(this);

    // Default to red and then yellow for following lights
    dlg->setEntry(nullptr, ' ');

    if(dlg->exec() == QDialog::Accepted && dlg && dlg->getLight())
    {
        SignalIndicatorListModel::LightEntry entry{dlg->getLight(), dlg->getLetter()};

        // Insert after current row or as last row
        const QModelIndex idx = mView->currentIndex();
        const int row = idx.isValid() ? idx.row() + 1 : mModel->rowCount();
        mModel->addEntryAt(row, entry);
    }

    delete dlg;
}

void SignalIndicatorListView::onRemove()
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    mModel->removeEntryAt(idx.row());
}

void SignalIndicatorListView::onEdit()
{
    const QModelIndex idx = mView->currentIndex();
    editIndex(idx);
}

void SignalIndicatorListView::editIndex(const QModelIndex &idx)
{
    if(!idx.isValid())
        return;

    QPointer<SignalIndicatorEntryDialog> dlg = new SignalIndicatorEntryDialog(this);
    SignalIndicatorListModel::LightEntry entry = mModel->getEntryAt(idx.row());
    dlg->setEntry(entry.light, entry.letter);

    if(dlg->exec() == QDialog::Accepted && dlg && dlg->getLight())
    {
        entry.light = dlg->getLight();
        entry.letter = dlg->getLetter();
        mModel->setEntryAt(idx.row(), entry);
    }

    delete dlg;
}

void SignalIndicatorListView::onMove(bool up)
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    mModel->moveRow(idx.row(), up);
}
