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

#include "utils/doubleclickslider.h"
#include <QDoubleSpinBox>

#include <QStandardPaths>
#include <QSettings>

#include <QJsonDocument>
#include <QJsonObject>

#include <QCloseEvent>

#include <QListView>
#include <QSortFilterProxyModel>


#include "utils/zoomgraphview.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>

#include <kddockwidgets-qt6/kddockwidgets/DockWidget.h>

MainWindow::MainWindow(const QString& uniqueName_, QWidget *parent)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName_, {}, parent)
{
    locateAppSettings();

    mRelaisModel = new RelaisModel(this);

    QSortFilterProxyModel *relaysProxy = new QSortFilterProxyModel(this);
    relaysProxy->setSourceModel(mRelaisModel);
    relaysProxy->setSortRole(Qt::DisplayRole);
    relaysProxy->sort(0);

    mRelaisView = new QListView;
    mRelaisView->setModel(relaysProxy);

    auto relaisDock = new KDDockWidgets::QtWidgets::DockWidget(QLatin1String("relais1"));
    relaisDock->setWidget(mRelaisView);
    relaisDock->asDockWidgetController()
    addDockWidget(relaisDock, KDDockWidgets::Location_OnRight);

    // connect(ui->addRelayBut, &QPushButton::clicked,
    //         mRelaisModel, [this]()
    // {
    //     QString name = QInputDialog::getText(this,
    //                                          tr("New Relay"),
    //                                          tr("Choose Name"));
    //     if(name.isEmpty())
    //         return;

    //     AbstractRelais *relay = new AbstractRelais(mRelaisModel);
    //     relay->setName(name);

    //     mRelaisModel->addRelay(relay);
    // });

    // connect(ui->removeRelayBut, &QPushButton::clicked,
    //         ui->relaisView, [this]()
    // {
    //     QModelIndex idx = ui->relaisView->currentIndex();
    //     if(!idx.isValid())
    //         return;

    //     AbstractRelais *relay = mRelaisModel->relayAt(idx.row());
    //     if(!relay)
    //         return;

    //     QString name = relay->name();
    //     int ret = QMessageBox::question(this,
    //                                     tr("Delete Relais %1").arg(name),
    //                                     tr("Are you sure to delete <b>%1</b>?").arg(name));
    //     if(ret == QMessageBox::Yes)
    //     {
    //         mRelaisModel->removeRelay(relay);
    //     }
    // });

    mEditFactory = new NodeEditFactory(this);
    StandardNodeTypes::registerTypes(mEditFactory);

    mScene = new CircuitScene(this);
    mScene->setRelaisModel(mRelaisModel);

    mCircuitView = new ZoomGraphView;
    mCircuitView->setScene(mScene);

    mZoomSlider = new DoubleClickSlider(Qt::Horizontal);
    mZoomSlider->setRange(0, ZoomGraphView::MaxZoom * 100);
    mZoomSlider->setTickInterval(25);
    mZoomSlider->setTickPosition(QSlider::TicksBothSides);
    statusBar()->addPermanentWidget(mZoomSlider);

    mZoomSpin = new QDoubleSpinBox;
    mZoomSpin->setRange(ZoomGraphView::MinZoom * 100,
                        ZoomGraphView::MaxZoom * 100);
    statusBar()->addPermanentWidget(mZoomSpin);


    auto circuitDock = new KDDockWidgets::QtWidgets::DockWidget(QLatin1String("circuit1"));
    circuitDock->setWidget(mCircuitView);
    addDockWidget(circuitDock, KDDockWidgets::Location_OnLeft);

    connect(mScene, &CircuitScene::nodeEditRequested,
            this, &MainWindow::nodeEditRequested);
    connect(mScene, &CircuitScene::cableEditRequested,
            this, &MainWindow::cableEditRequested);

    connect(mRelaisModel, &RelaisModel::modelEdited, this, &MainWindow::updateWindowModified);
    connect(mScene, &CircuitScene::sceneEdited, this, &MainWindow::updateWindowModified);

    connect(mCircuitView, &ZoomGraphView::zoomChanged,
            this, &MainWindow::onZoomChanged);
    connect(mZoomSlider, &QSlider::valueChanged,
            this, &MainWindow::onZoomSliderChanged);
    connect(mZoomSlider, &DoubleClickSlider::sliderHandleDoubleClicked,
            this, &MainWindow::resetZoom);
    connect(mZoomSpin, &QDoubleSpinBox::valueChanged,
            this, &MainWindow::onZoomSpinChanged);

    onZoomChanged(mCircuitView->zoomFactor());

    buildToolBar();
}

MainWindow::~MainWindow()
{
    // These classes emit signals in destructors which
    // would happen after MainWindow destructor, in super class.
    // Delete them now.
    delete mRelaisModel;
    delete mScene;
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
    // Menu File
    QMenu *menuFile = menuBar()->addMenu(tr("File"));

    actionNew = menuFile->addAction(tr("&New"));
    actionOpen = menuFile->addAction(tr("&Open"));
    actionOpen_Recent = menuFile->addAction(tr("Open &Recent"));

    actionSave = menuFile->addAction(tr("&Save"));
    actionSave->setShortcut(tr("Ctrl+S"));

    actionSave_As = menuFile->addAction(tr("Sa&ve As"));
    actionSave_As->setShortcut(tr("Ctrl+Shift+S"));

    connect(actionNew, &QAction::triggered,
            this, &MainWindow::onNew);
    connect(actionOpen, &QAction::triggered,
            this, &MainWindow::onOpen);
    connect(actionSave, &QAction::triggered,
            this, &MainWindow::onSave);
    connect(actionSave_As, &QAction::triggered,
            this, &MainWindow::onSaveAs);

    // Recent files
    QMenu *recentFilesMenu = new QMenu(this);
    for (int i = 0; i < MaxRecentFiles; i++)
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], &QAction::triggered, this, &MainWindow::onOpenRecent);

        recentFilesMenu->addAction(recentFileActs[i]);
    }
    actionOpen_Recent->setMenu(recentFilesMenu);

    updateRecentFileActions();

    // Toolbar
    QToolBar *toolBar = new QToolBar(tr("Edit Tools"));
    addToolBar(Qt::TopToolBarArea, toolBar);

    QAction *toggleEditMode = new QAction(tr("Edit"));
    toggleEditMode->setCheckable(true);
    toggleEditMode->setChecked(mScene->mode() == CircuitScene::Mode::Editing);
    toolBar->addAction(toggleEditMode);

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
    toolBar->addAction(newItem);
    toolBar->addSeparator();

    QVector<QAction *> addItemActions;

    for(const QString& nodeType : mEditFactory->getRegisteredTypes())
    {
        QString title = tr("New %1").arg(mEditFactory->prettyName(nodeType));

        QAction *act = newItemMenu->addAction(title);
        connect(act, &QAction::triggered, mScene,
                [nodeType, this]()
        {
            //ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->circuitTab));

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
        //ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->circuitTab));

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
        toolBar->addAction(act);
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
    setWindowModified(false);
    setWindowFilePath(QString());

    mIsLoading = true;

    mScene->removeAllItems();
    mScene->setHasUnsavedChanges(false);

    mRelaisModel->clear();
    mRelaisModel->setHasUnsavedChanges(false);

    mIsLoading = false;
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

    mIsLoading = true;

    setWindowFilePath(fileName);
    updateWindowModified();

    addFileToRecents(fileName);

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    const QJsonObject rootObj = doc.object();

    const QJsonObject relais = rootObj.value("relais_model").toObject();
    mRelaisModel->loadFromJSON(relais);

    const QJsonObject sceneObj = rootObj.value("scene").toObject();
    mScene->loadFromJSON(sceneObj, mEditFactory);

    mIsLoading = false;
    updateWindowModified();
}

void MainWindow::locateAppSettings()
{
    QString p = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    p.append("/settings.ini");
    settingsFile = QDir::cleanPath(p);
}

bool MainWindow::hasUnsavedChanges() const
{
    if(mIsLoading)
        return false;

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
        if(!saveFile(windowFilePath()))
            return false; // Saving was canceled
    }

    return true;
}

bool MainWindow::saveFile(const QString& fileName)
{
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

    addFileToRecents(fileName);

    mScene->setHasUnsavedChanges(false);
    mRelaisModel->setHasUnsavedChanges(false);

    return true;
}

void MainWindow::onSave()
{
    if(windowFilePath().isEmpty())
    {
        // Show file dialog for new files
        onSaveAs();
        return;
    }

    // Save on same file
    saveFile(windowFilePath());
}

void MainWindow::onSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Circuit"),
                                                    windowFilePath());
    if(fileName.isEmpty())
        return;

    saveFile(fileName);

    // Set current file to new file path
    setWindowFilePath(fileName);
}

void MainWindow::updateWindowModified()
{
    // Do not set modified state for new files
    if(windowFilePath().isEmpty())
        setWindowModified(false);
    else
        setWindowModified(hasUnsavedChanges());
}

void MainWindow::onZoomChanged(double val)
{
    QSignalBlocker blk(mZoomSpin);
    mZoomSpin->setValue(val * 100.0);

    QSignalBlocker blk2(mZoomSlider);
    mZoomSlider->setValue(qRound(mZoomSpin->value()));
}

void MainWindow::onZoomSliderChanged(int val)
{
    mCircuitView->setZoom(double(val) / 100.0);
}

void MainWindow::onZoomSpinChanged(double val)
{
    mCircuitView->setZoom(val / 100.0);
}

void MainWindow::resetZoom()
{
    mCircuitView->setZoom(1.0);
}

