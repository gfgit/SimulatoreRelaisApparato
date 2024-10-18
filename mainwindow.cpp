#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "graph/circuitscene.h"

#include "nodes/onoffswitchnode.h"
#include "nodes/powersourcenode.h"
#include "nodes/simplecircuitnode.h"
#include "nodes/relaispowernode.h"
#include "nodes/relaiscontactnode.h"
#include "circuitcable.h"
#include "abstractrelais.h"

#include "graph/cablegraphitem.h"
#include "graph/onoffgraphitem.h"
#include "graph/powersourcegraphitem.h"
#include "graph/relaispowergraphitem.h"
#include "graph/relaiscontactgraphitem.h"
#include "graph/simplenodegraphitem.h"

#include <QAction>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mScene = new CircuitScene(this);
    ui->graphicsView->setScene(mScene);

    connect(mScene, &CircuitScene::nodeEditRequested,
            this, &MainWindow::nodeEditRequested);
    connect(mScene, &CircuitScene::cableEditRequested,
            this, &MainWindow::cableEditRequested);

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

template <typename Node, typename Graph>
Node* addNewNodeToScene(CircuitScene *s, QGraphicsView *v)
{
    Node *node = new Node(s);
    Graph *graph = new Graph(node);
    s->addNode(graph);

    v->ensureVisible(graph);

    return node;
}

template <typename Graph>
void addNewNodeToMenu(QMenu *menu, const QString& title, CircuitScene *s, QGraphicsView *v)
{
    QAction *act = menu->addAction(title);
    QObject::connect(act, &QAction::triggered, s,
                     [s, v, menu]()
    {
        QString name = QInputDialog::getText(v,
                                             MainWindow::tr("Choose Item Name"),
                                             MainWindow::tr("Name:"));
        if(name.isEmpty())
            return;

        typename Graph::Node *node = addNewNodeToScene<typename Graph::Node, Graph>(s, v);
        node->setObjectName(name);
    });
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

    addNewNodeToMenu<OnOffGraphItem>(newItemMenu,
                                     tr("New On/Off Switch"),
                                     mScene,
                                     ui->graphicsView);
    addNewNodeToMenu<PowerSourceGraphItem>(newItemMenu,
                                     tr("New Power Source"),
                                     mScene,
                                     ui->graphicsView);
    addNewNodeToMenu<RelaisContactGraphItem>(newItemMenu,
                                     tr("New Relay Contact"),
                                     mScene,
                                     ui->graphicsView);
    addNewNodeToMenu<RelaisPowerGraphItem>(newItemMenu,
                                     tr("New Relay Power"),
                                     mScene,
                                     ui->graphicsView);
    addNewNodeToMenu<SimpleNodeGraphItem>(newItemMenu,
                                     tr("New Simple Node"),
                                     mScene,
                                     ui->graphicsView);

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
}

void MainWindow::cableEditRequested(CableGraphItem *item)
{
    // Allow delete or modify path
}

