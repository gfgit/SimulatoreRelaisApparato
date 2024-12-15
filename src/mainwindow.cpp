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

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>

#include <QStandardPaths>
#include <QSettings>

#include <QJsonDocument>
#include <QJsonObject>

#include <QCloseEvent>

#include <QMenuBar>
#include <QToolBar>

#include "views/viewmanager.h"
#include "views/modemanager.h"

#include "circuits/edit/nodeeditfactory.h"
#include "panels/edit/panelitemfactory.h"
#include "objects/simulationobjectfactory.h"

static constexpr const char *allFiles =
        QT_TRANSLATE_NOOP("MainWindow", "All Files (*.*)");
static constexpr const char *jsonFiles =
        QT_TRANSLATE_NOOP("MainWindow", "JSON Files (*.json)");
static constexpr const char *simraFormat =
  QT_TRANSLATE_NOOP("MainWindow", "Simulatore Relais Circuits (*.simrelaisc)");

MainWindow::MainWindow(const QString& uniqueName_, const QString& settingsFile_, QWidget *parent)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName_, {}, parent)
    , settingsFile(settingsFile_)
{
    mModeMgr = new ModeManager(this);

    mViewMgr = new ViewManager(this);

    connect(mModeMgr, &ModeManager::modeChanged,
            this, &MainWindow::onFileModeChanged);
    connect(mModeMgr, &ModeManager::fileEdited,
            this, &MainWindow::updateWindowModified);

    connect(mViewMgr, &ViewManager::currentViewTypeChanged,
            this, &MainWindow::showCurrentEditToolbars);

    buildMenuBar();

    resize(800, 600);

    // Start with a new file
    onNew();
}

MainWindow::~MainWindow()
{
    // These classes emit signals in destructors which
    // would happen after MainWindow destructor, in super class.
    // Delete them now.
    delete mViewMgr;
    mViewMgr = nullptr;

    delete mModeMgr;
    mModeMgr = nullptr;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if(maybeSave())
        e->accept();
    else
        e->ignore();
}

void MainWindow::buildMenuBar()
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


    // Menu View
    QMenu *menuView = menuBar()->addMenu(tr("View"));

    QAction *showCircuitList = menuView->addAction(tr("Circuit list"));
    connect(showCircuitList, &QAction::triggered,
            mViewMgr, &ViewManager::showCircuitListView);

    QAction *showPanelList = menuView->addAction(tr("Panel list"));
    connect(showPanelList, &QAction::triggered,
            mViewMgr, &ViewManager::showPanelListView);

    QMenu *menuViewObject = menuView->addMenu(tr("Object List"));

    SimulationObjectFactory *factory = modeMgr()->objectFactory();
    for(const QString& objType : factory->getRegisteredTypes())
    {
        const QString prettyName = factory->prettyName(objType);

        QAction *listAction = menuViewObject->addAction(prettyName);

        connect(listAction, &QAction::triggered,
                mViewMgr, [this, objType]()
        {
            mViewMgr->showObjectListView(objType);
        });
    }

    actionEditMode = menuView->addAction(tr("Edit mode"));
    actionEditMode->setCheckable(true);

    connect(actionEditMode, &QAction::toggled,
            mModeMgr, [this](bool val)
    {
        // If was Editing set to Simulation when unchecked
        // Otherwise leave current mode on
        const bool isEditing = mModeMgr->mode() == FileMode::Editing;
        if(val && !isEditing)
            mModeMgr->setMode(FileMode::Editing);
        else if(!val && isEditing)
            mModeMgr->setMode(FileMode::Simulation);
    });

    if(mModeMgr->mode() == FileMode::Editing)
    {
        actionEditMode->setChecked(true);
        buildEditToolBar();
    }
}

void MainWindow::buildEditToolBar()
{
    // Circuits
    Q_ASSERT(!circuitEditToolbar1 && !circuitEditToolbar2);

    // Use 2 Toolbars to show actions in 2 rows
    circuitEditToolbar1 = new QToolBar(tr("Circuit Tools 1"));
    circuitEditToolbar2 = new QToolBar(tr("Circuit Tools 2"));
    addToolBar(Qt::TopToolBarArea, circuitEditToolbar1);
    insertToolBar(circuitEditToolbar1, circuitEditToolbar2);
    insertToolBarBreak(circuitEditToolbar1);

    QAction *newCircuitItem = new QAction(tr("New Circuit Item"));
    QMenu *newCircuitItemMenu = new QMenu;
    newCircuitItem->setMenu(newCircuitItemMenu);
    circuitEditToolbar1->addAction(newCircuitItem);
    circuitEditToolbar1->addSeparator();

    QVector<QAction *> addCircuitItemActions;

    auto circuitEditFactory = mModeMgr->circuitFactory();
    for(const QString& nodeType : circuitEditFactory->getRegisteredTypes())
    {
        QString title = tr("%1").arg(circuitEditFactory->prettyName(nodeType));

        QAction *act = newCircuitItemMenu->addAction(title);
        connect(act, &QAction::triggered, mViewMgr,
                [nodeType, this]()
        {
            mViewMgr->addNodeToActiveView(nodeType);
        });

        addCircuitItemActions.append(act);
    }

    QAction *newCableAct = newCircuitItemMenu->addAction(tr("Cable"));
    connect(newCableAct, &QAction::triggered,
            mViewMgr, &ViewManager::startEditNewCableOnActiveView);

    addCircuitItemActions.append(newCableAct);

    int i = 0;
    for(QAction *act : std::as_const(addCircuitItemActions))
    {
        if(i < addCircuitItemActions.size() / 2)
            circuitEditToolbar1->addAction(act);
        else
            circuitEditToolbar2->addAction(act);
        i++;
    }

    // Panels
    Q_ASSERT(!panelEditToolbar1);

    panelEditToolbar1 = new QToolBar(tr("Panel Tools 1"));
    insertToolBarBreak(circuitEditToolbar2);
    insertToolBar(circuitEditToolbar2, panelEditToolbar1);

    QAction *newPanelItem = new QAction(tr("New Panel Item"));
    QMenu *newPanelItemMenu = new QMenu;
    newPanelItem->setMenu(newPanelItemMenu);
    panelEditToolbar1->addAction(newPanelItem);
    panelEditToolbar1->addSeparator();

    QVector<QAction *> addPanelItemActions;

    auto panelEditFactory = mModeMgr->panelFactory();
    for(const QString& nodeType : panelEditFactory->getRegisteredTypes())
    {
        QString title = tr("%1").arg(panelEditFactory->prettyName(nodeType));

        QAction *act = newPanelItemMenu->addAction(title);
        connect(act, &QAction::triggered, mViewMgr,
                [nodeType, this]()
        {
            mViewMgr->addNodeToActiveView(nodeType);
        });

        addPanelItemActions.append(act);
    }

    for(QAction *act : std::as_const(addPanelItemActions))
    {
        panelEditToolbar1->addAction(act);
    }

    showCurrentEditToolbars();
}

void MainWindow::removeEditToolBar()
{
    Q_ASSERT(circuitEditToolbar1 && circuitEditToolbar2);

    removeToolBarBreak(circuitEditToolbar1);

    removeToolBar(circuitEditToolbar1);
    delete circuitEditToolbar1;
    circuitEditToolbar1 = nullptr;

    removeToolBarBreak(circuitEditToolbar2);

    removeToolBar(circuitEditToolbar2);
    delete circuitEditToolbar2;
    circuitEditToolbar2 = nullptr;

    Q_ASSERT(panelEditToolbar1);

    removeToolBar(panelEditToolbar1);
    delete panelEditToolbar1;
    panelEditToolbar1 = nullptr;
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

void MainWindow::onNew()
{
    if(!maybeSave())
        return;

    setWindowModified(false);
    setWindowFilePath(QString());
    mModeMgr->setFilePath(QString());

    // Reset scenes and objects
    mViewMgr->closeAllFileSpecificDocks();
    mModeMgr->clearAll();

    // Show circuit list view for new files
    mViewMgr->showCircuitListView();
}

void MainWindow::onOpen()
{
    if(!maybeSave())
        return;

    QStringList filters = {simraFormat, jsonFiles, allFiles};
    for(auto &s : filters)
        s = tr(s.toLatin1());
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Circuit"),
                                                    QString(),
                                                    filters.join(QLatin1String(";;")));
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

    mViewMgr->closeAllFileSpecificDocks();

    setWindowFilePath(fileName);
    mModeMgr->setFilePath(fileName);
    updateWindowModified();

    addFileToRecents(fileName);

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if(doc.isNull())
        return;

    const QJsonObject rootObj = doc.object();

    mModeMgr->loadFromJSON(rootObj);

    updateWindowModified();

    // Show circuit list view for opened files
    mViewMgr->showCircuitListView();
}

bool MainWindow::maybeSave()
{
    if(!mModeMgr->fileNeedsSaving())
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
        if(!onSave())
            return false; // Saving was canceled
    }

    return true;
}

bool MainWindow::saveFile(const QString& fileName)
{
    // Set before saving to allow getting relative path
    const QString oldFilePath = mModeMgr->filePath();
    mModeMgr->setFilePath(fileName);

    QJsonObject rootObj;
    mModeMgr->saveToJSON(rootObj);

    // Reset
    mModeMgr->setFilePath(oldFilePath);

    QJsonDocument doc(rootObj);

    QFile f(fileName);
    if(!f.open(QFile::WriteOnly))
        return false;

    f.write(doc.toJson());

    addFileToRecents(fileName);

    mModeMgr->resetFileEdited();

    return true;
}

bool MainWindow::onSave()
{
    if(windowFilePath().isEmpty())
    {
        // Show file dialog for new files
        return onSaveAs();
    }

    // Save on same file
    return saveFile(windowFilePath());
}

bool MainWindow::onSaveAs()
{
    QStringList filters = {simraFormat, jsonFiles, allFiles};
    for(auto &s : filters)
        s = tr(s.toLatin1());
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Circuit"),
                                                    windowFilePath(),
                                                    filters.join(QLatin1String(";;")));
    if(fileName.isEmpty())
        return false;

    if(!saveFile(fileName))
        return false;

    // Set current file to new file path
    setWindowFilePath(fileName);
    mModeMgr->setFilePath(fileName);
    return true;
}

void MainWindow::updateWindowModified()
{
    // Do not set modified state for new files
    if(windowFilePath().isEmpty())
        setWindowModified(false);
    else
        setWindowModified(mModeMgr->fileNeedsSaving());
}

ModeManager *MainWindow::modeMgr() const
{
    return mModeMgr;
}

void MainWindow::onFileModeChanged(FileMode mode, FileMode oldMode)
{
    actionEditMode->setChecked(mode == FileMode::Editing);

    if(mode == FileMode::Editing)
        buildEditToolBar();
    else if(oldMode == FileMode::Editing)
        removeEditToolBar();
}

void MainWindow::showCurrentEditToolbars()
{
    if(mModeMgr->mode() != FileMode::Editing)
        return;

    ViewManager::ViewType currViewType = mViewMgr->currentViewType();

    const bool canShowCircuits = currViewType == ViewManager::ViewType::Circuit;
    const bool canEditCircuits = mViewMgr->activeCircuitView() != nullptr;
    if(circuitEditToolbar1)
    {
        circuitEditToolbar1->setVisible(canShowCircuits);
        circuitEditToolbar1->setEnabled(canEditCircuits);
    }
    if(circuitEditToolbar2)
    {
        circuitEditToolbar2->setVisible(canShowCircuits);
        circuitEditToolbar2->setEnabled(canEditCircuits);
    }

    const bool canShowPanels = currViewType == ViewManager::ViewType::Panel;
    const bool canEditPanels = mViewMgr->activePanelView() != nullptr;
    if(panelEditToolbar1)
    {
        panelEditToolbar1->setVisible(canShowPanels);
        panelEditToolbar1->setEnabled(canEditPanels);
    }
}
