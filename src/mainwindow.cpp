/**
 * src/mainwindow.cpp
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

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "graph/circuitscene.h"

#include "nodes/abstractcircuitnode.h"
#include "graph/abstractnodegraphitem.h"

#include "objects/abstractrelais.h"
#include "objects/relaismodel.h"

#include "nodes/edit/nodeeditfactory.h"
#include "nodes/edit/standardnodetypes.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

#include <QStandardPaths>
#include <QSettings>

#include <QJsonDocument>
#include <QJsonObject>

#include <QCloseEvent>

#include <QSortFilterProxyModel>

#include "graph/zoomgraphview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    locateAppSettings();

    mRelaisModel = new RelaisModel(this);

    QSortFilterProxyModel *relaysProxy = new QSortFilterProxyModel(this);
    relaysProxy->setSourceModel(mRelaisModel);
    relaysProxy->setSortRole(Qt::DisplayRole);
    relaysProxy->sort(0);
    ui->relaisView->setModel(relaysProxy);

    connect(ui->addRelayBut, &QPushButton::clicked,
            mRelaisModel, [this]()
    {
        QString name = QInputDialog::getText(this,
                                             tr("New Relay"),
                                             tr("Choose Name"));
        if(name.isEmpty())
            return;

        AbstractRelais *relay = new AbstractRelais(mRelaisModel);
        relay->setName(name);

        mRelaisModel->addRelay(relay);
    });

    connect(ui->removeRelayBut, &QPushButton::clicked,
            ui->relaisView, [this]()
    {
        QModelIndex idx = ui->relaisView->currentIndex();
        if(!idx.isValid())
            return;

        AbstractRelais *relay = mRelaisModel->relayAt(idx.row());
        if(!relay)
            return;

        QString name = relay->name();
        int ret = QMessageBox::question(this,
                                        tr("Delete Relais %1").arg(name),
                                        tr("Are you sure to delete <b>%1</b>?").arg(name));
        if(ret == QMessageBox::Yes)
        {
            mRelaisModel->removeRelay(relay);
        }
    });

    mEditFactory = new NodeEditFactory(this);
    StandardNodeTypes::registerTypes(mEditFactory);

    mScene = new CircuitScene(this);
    mScene->setRelaisModel(mRelaisModel);

    mCircuitView = new ZoomGraphView;
    mCircuitView->setScene(mScene);

    QVBoxLayout *circuitLay = new QVBoxLayout(ui->circuitTab);
    circuitLay->addWidget(mCircuitView);

    connect(mScene, &CircuitScene::nodeEditRequested,
            this, &MainWindow::nodeEditRequested);
    connect(mScene, &CircuitScene::cableEditRequested,
            this, &MainWindow::cableEditRequested);

    connect(ui->actionNew, &QAction::triggered,
            this, &MainWindow::onNew);
    connect(ui->actionOpen, &QAction::triggered,
            this, &MainWindow::onOpen);
    connect(ui->actionSave, &QAction::triggered,
            this, &MainWindow::onSave);

    QMenu *recentFilesMenu = new QMenu(this);
    for (int i = 0; i < MaxRecentFiles; i++)
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], &QAction::triggered, this, &MainWindow::onOpenRecent);

        recentFilesMenu->addAction(recentFileActs[i]);
    }

    updateRecentFileActions();

    ui->actionOpen_Recent->setMenu(recentFilesMenu);

    buildToolBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

CircuitScene *MainWindow::scene() const
{
    return mScene;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if(maybeSave())
        e->accept();
    else
        e->ignore();
}

void MainWindow::buildToolBar()
{
    QAction *toggleEditMode = new QAction(tr("Edit"));
    toggleEditMode->setCheckable(true);
    toggleEditMode->setChecked(mScene->mode() == CircuitScene::Mode::Editing);
    ui->toolBar->addAction(toggleEditMode);

    connect(toggleEditMode, &QAction::toggled,
            this, [this](bool val)
    {
        // If was Editing set to Simulation when unchecked
        // Otherwise leave current mode on
        const bool isEditing = mScene->mode() == CircuitScene::Mode::Editing;
        if(val && !isEditing)
            mScene->setMode(CircuitScene::Mode::Editing);
        else if(!val && isEditing)
            mScene->setMode(CircuitScene::Mode::Simulation);
    });

    QAction *newItem = new QAction(tr("New Item"));
    QMenu *newItemMenu = new QMenu;
    newItem->setMenu(newItemMenu);
    ui->toolBar->addAction(newItem);
    ui->toolBar->addSeparator();

    QVector<QAction *> addItemActions;

    for(const QString& nodeType : mEditFactory->getRegisteredTypes())
    {
        QString title = tr("New %1").arg(mEditFactory->prettyName(nodeType));

        QAction *act = newItemMenu->addAction(title);
        connect(act, &QAction::triggered, mScene,
                [nodeType, this]()
        {
            ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->circuitTab));

            const auto needsName = mEditFactory->needsName(nodeType);
            QString name;
            if(needsName == NodeEditFactory::NeedsName::Always)
            {
                name = QInputDialog::getText(mCircuitView,
                                             MainWindow::tr("Choose Item Name"),
                                             MainWindow::tr("Name:"));
                if(name.isEmpty())
                    return;
            }

            QPoint vpCenter = mCircuitView->viewport()->rect().center();
            QPointF sceneCenter = mCircuitView->mapToScene(vpCenter);
            TileLocation hint = TileLocation::fromPoint(sceneCenter);

            auto item = mEditFactory->createItem(nodeType, mScene, hint);
            if(needsName == NodeEditFactory::NeedsName::Always)
                item->getAbstractNode()->setObjectName(name);

            mCircuitView->ensureVisible(item);
        });

        addItemActions.append(act);
    }

    QAction *newCableAct = newItemMenu->addAction(tr("New Cable"));
    connect(newCableAct, &QAction::triggered, mScene,
            [this]()
    {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->circuitTab));

        mScene->startEditNewCable();
    });

    addItemActions.append(newCableAct);

    connect(mScene, &CircuitScene::modeChanged,
            this, [this, toggleEditMode, newItem, addItemActions](CircuitScene::Mode mode)
    {
        toggleEditMode->setChecked(mode == CircuitScene::Mode::Editing);
        newItem->setEnabled(mode == CircuitScene::Mode::Editing);

        for(QAction *act : std::as_const(addItemActions))
        {
            act->setVisible(mode == CircuitScene::Mode::Editing);
        }
    });

    for(QAction *act : std::as_const(addItemActions))
    {
        ui->toolBar->addAction(act);
    }
}

static QString strippedName(const QString &fullFileName, bool *ok)
{
    QFileInfo fi(fullFileName);
    if (ok)
        *ok = fi.exists();
    return fi.fileName();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(settingsFile, QSettings::IniFormat);

    QStringList files = settings.value("recent_files").toStringList();

    int numRecentFiles = qMin(files.size(), int(MaxRecentFiles));

    for (int i = 0; i < numRecentFiles; i++)
    {
        bool ok      = true;
        QString name = strippedName(files[i], &ok);
        if (name.isEmpty() || !ok)
        {
            files.removeAt(i);
            i--;
            numRecentFiles = qMin(files.size(), int(MaxRecentFiles));
        }
        else
        {
            QString text = tr("&%1 %2").arg(i + 1).arg(name);
            recentFileActs[i]->setText(text);
            recentFileActs[i]->setData(files[i]);
            recentFileActs[i]->setToolTip(files[i]);
            recentFileActs[i]->setVisible(true);
        }
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    settings.setValue("recent_files", files);
}

void MainWindow::addFileToRecents(const QString &fileName)
{
    // Store in recent files
    QSettings settings(settingsFile, QSettings::IniFormat);
    QStringList files = settings.value("recent_files").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recent_files", files);

    updateRecentFileActions();
}

void MainWindow::nodeEditRequested(AbstractNodeGraphItem *item)
{
    // Allow delete or custom node options
    mEditFactory->editItem(this, item);
}

void MainWindow::cableEditRequested(CableGraphItem *item)
{
    // Allow delete or modify path
    mEditFactory->editCable(this, item);
}

void MainWindow::onNew()
{
    if(!maybeSave())
        return;

    // Reset scene and relais
    setWindowFilePath(QString());
    mScene->removeAllItems();
    mScene->setHasUnsavedChanges(false);

    mRelaisModel->clear();
    mRelaisModel->setHasUnsavedChanges(false);
}

void MainWindow::onOpen()
{
    if(!maybeSave())
        return;

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Circuit"));
    if(fileName.isEmpty())
        return;

    loadFile(fileName);
}

void MainWindow::onOpenRecent()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (!act)
        return;

    if(!maybeSave())
        return;

    loadFile(act->data().toString());
}

void MainWindow::loadFile(const QString& fileName)
{
    QFile f(fileName);
    if(!f.open(QFile::ReadOnly))
        return;

    setWindowFilePath(fileName);

    addFileToRecents(fileName);

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    const QJsonObject rootObj = doc.object();

    const QJsonObject relais = rootObj.value("relais_model").toObject();
    mRelaisModel->loadFromJSON(relais);

    const QJsonObject sceneObj = rootObj.value("scene").toObject();
    mScene->loadFromJSON(sceneObj, mEditFactory);
}

void MainWindow::locateAppSettings()
{
    QString p = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    p.append("/settings.ini");
    settingsFile = QDir::cleanPath(p);
}

bool MainWindow::hasUnsavedChanges() const
{
    if(mScene->hasUnsavedChanges())
        return true;

    if(mRelaisModel->hasUnsavedChanges())
        return true;

    return false;
}

bool MainWindow::maybeSave()
{
    if(!hasUnsavedChanges())
        return true; // No need to save

    QMessageBox::StandardButton ret =
            QMessageBox::question(this,
                                  tr("Save Changes?"),
                                  tr("Current file has unsaved changes.\n"
                                     "Do you want to save them now?"),
                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                  QMessageBox::Save);

    if(ret == QMessageBox::Cancel)
        return false; // Keep current file open

    if(ret == QMessageBox::Save)
    {
        // Check if really save
        if(!saveInternal())
            return false; // Saving was canceled
    }

    return true;
}

bool MainWindow::saveInternal()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Circuit"),
                                                    windowFilePath());
    if(fileName.isEmpty())
        return false;

    QJsonObject relais;
    mRelaisModel->saveToJSON(relais);

    QJsonObject sceneObj;
    mScene->saveToJSON(sceneObj);

    QJsonObject rootObj;
    rootObj["relais_model"] = relais;
    rootObj["scene"] = sceneObj;

    QJsonDocument doc(rootObj);

    QFile f(fileName);
    if(!f.open(QFile::WriteOnly))
        return false;

    f.write(doc.toJson());

    setWindowFilePath(fileName);
    addFileToRecents(fileName);

    mScene->setHasUnsavedChanges(false);
    mRelaisModel->setHasUnsavedChanges(false);

    return true;
}

void MainWindow::onSave()
{
    saveInternal();
}

