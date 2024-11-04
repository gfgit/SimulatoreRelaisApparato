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

#include "nodes/edit/nodeeditfactory.h"

MainWindow::MainWindow(const QString& uniqueName_, QWidget *parent)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName_, {}, parent)
{
    locateAppSettings();

    mModeMgr = new ModeManager(this);

    mViewMgr = new ViewManager(this);

    connect(mModeMgr, &ModeManager::modeChanged,
            this, &MainWindow::onFileModeChanged);
    connect(mModeMgr, &ModeManager::fileEdited,
            this, &MainWindow::updateWindowModified);

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

    QAction *showRelayList = menuView->addAction(tr("Relay list"));

    connect(showRelayList, &QAction::triggered,
            mViewMgr, &ViewManager::showRelayListView);

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
    Q_ASSERT(!editToolbar1 && !editToolbar2);

    // Use 2 Toolbars to show actions in 2 rows
    editToolbar1 = new QToolBar(tr("Edit Tools 1"));
    editToolbar2 = new QToolBar(tr("Edit Tools 2"));
    addToolBar(Qt::TopToolBarArea, editToolbar1);
    insertToolBar(editToolbar1, editToolbar2);
    insertToolBarBreak(editToolbar1);

    QAction *newItem = new QAction(tr("New Item"));
    QMenu *newItemMenu = new QMenu;
    newItem->setMenu(newItemMenu);
    editToolbar1->addAction(newItem);
    editToolbar1->addSeparator();

    QVector<QAction *> addItemActions;

    auto editFactory = mModeMgr->circuitFactory();
    for(const QString& nodeType : editFactory->getRegisteredTypes())
    {
        QString title = tr("New %1").arg(editFactory->prettyName(nodeType));

        QAction *act = newItemMenu->addAction(title);
        connect(act, &QAction::triggered, mViewMgr,
                [nodeType, this]()
        {
            mViewMgr->addNodeToActiveView(nodeType);
        });

        addItemActions.append(act);
    }

    QAction *newCableAct = newItemMenu->addAction(tr("New Cable"));
    connect(newCableAct, &QAction::triggered,
            mViewMgr, &ViewManager::startEditNEwCableOnActiveView);

    addItemActions.append(newCableAct);

    int i = 0;
    for(QAction *act : std::as_const(addItemActions))
    {
        if(i < 4)
            editToolbar1->addAction(act);
        else
            editToolbar2->addAction(act);
        i++;
    }
}

void MainWindow::removeEditToolBar()
{
    Q_ASSERT(editToolbar1 && editToolbar2);

    removeToolBarBreak(editToolbar1);

    removeToolBar(editToolbar1);
    delete editToolbar1;
    editToolbar1 = nullptr;

    removeToolBar(editToolbar2);
    delete editToolbar2;
    editToolbar2 = nullptr;
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

    mViewMgr->closeAllFileSpecificDocks();

    setWindowFilePath(fileName);
    updateWindowModified();

    addFileToRecents(fileName);

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    const QJsonObject rootObj = doc.object();

    mModeMgr->loadFromJSON(rootObj);

    updateWindowModified();

    // Show circuit list view for opened files
    mViewMgr->showCircuitListView();
}

void MainWindow::locateAppSettings()
{
    QString p = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    p.append("/settings.ini");
    settingsFile = QDir::cleanPath(p);
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
        if(!saveFile(windowFilePath()))
            return false; // Saving was canceled
    }

    return true;
}

bool MainWindow::saveFile(const QString& fileName)
{
    QJsonObject rootObj;
    mModeMgr->saveToJSON(rootObj);

    QJsonDocument doc(rootObj);

    QFile f(fileName);
    if(!f.open(QFile::WriteOnly))
        return false;

    f.write(doc.toJson());

    addFileToRecents(fileName);

    mModeMgr->resetFileEdited();

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
