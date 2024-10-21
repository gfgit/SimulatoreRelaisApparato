#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "graph/circuitscene.h"

#include "abstractcircuitnode.h"
#include "graph/abstractnodegraphitem.h"

#include "abstractrelais.h"

#include "relaismodel.h"

#include "nodes/edit/nodeeditfactory.h"
#include "nodes/edit/standardnodetypes.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
            this, &MainWindow::loadFile);
    connect(ui->actionSave, &QAction::triggered,
            this, &MainWindow::saveFile);

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
        mScene->setMode(val ? CircuitScene::Mode::Editing :
                              CircuitScene::Mode::Simulation);
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

void MainWindow::loadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Circuit"));
    if(fileName.isEmpty())
        return;

    QFile f(fileName);
    if(!f.open(QFile::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    const QJsonObject rootObj = doc.object();

    const QJsonObject relais = rootObj.value("relais_model").toObject();
    mRelaisModel->loadFromJSON(relais);

    const QJsonObject sceneObj = rootObj.value("scene").toObject();
    mScene->loadFromJSON(sceneObj, mEditFactory);
}

void MainWindow::saveFile()
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

