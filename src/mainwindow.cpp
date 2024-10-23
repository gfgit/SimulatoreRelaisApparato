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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    locateAppSettings();

    mRelaisModel = new RelaisModel(this);
    ui->relaisView->setModel(mRelaisModel);

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

    ui->graphicsView->setScene(mScene);

    connect(mScene, &CircuitScene::nodeEditRequested,
            this, &MainWindow::nodeEditRequested);
    connect(mScene, &CircuitScene::cableEditRequested,
            this, &MainWindow::cableEditRequested);

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

    for(const QString& nodeType : mEditFactory->getRegisteredTypes())
    {
        QString title = tr("New %1").arg(mEditFactory->prettyName(nodeType));

        QAction *act = newItemMenu->addAction(title);
        connect(act, &QAction::triggered, mScene,
                [nodeType, this]()
        {
            ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->circuitTab));

            const bool needsName = mEditFactory->needsName(nodeType);
            QString name;
            if(needsName)
            {
                name = QInputDialog::getText(ui->graphicsView,
                                             MainWindow::tr("Choose Item Name"),
                                             MainWindow::tr("Name:"));
                if(name.isEmpty())
                    return;
            }

            auto item = mEditFactory->createItem(nodeType, mScene);
            if(needsName)
                item->getAbstractNode()->setObjectName(name);

            ui->graphicsView->centerOn(item->boundingRect().center());
        });
    }

    QAction *newCableAct = newItemMenu->addAction(tr("New Cable"));
    connect(newCableAct, &QAction::triggered, mScene,
                     [this]()
    {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->circuitTab));

        mScene->startEditNewCable();
    });

    connect(mScene, &CircuitScene::modeChanged,
            this, [this, toggleEditMode, newItem](CircuitScene::Mode mode)
    {
        toggleEditMode->setChecked(mode == CircuitScene::Mode::Editing);
        newItem->setEnabled(mode == CircuitScene::Mode::Editing);
    });
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
    QSettings settings(settingsFile);

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

void MainWindow::onOpen()
{
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

    loadFile(act->data().toString());
}

void MainWindow::loadFile(const QString& fileName)
{
    QFile f(fileName);
    if(!f.open(QFile::ReadOnly))
        return;

    setWindowFilePath(fileName);

    // Store in recent files
    QSettings settings(settingsFile);
    QStringList files = settings.value("recent_files").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recent_files", files);

    updateRecentFileActions();

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

void MainWindow::onSave()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Circuit"));
    if(fileName.isEmpty())
        return;

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
        return;

    f.write(doc.toJson());
}

