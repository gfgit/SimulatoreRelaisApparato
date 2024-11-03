#include "circuitlistwidget.h"

#include "circuitlistmodel.h"

#include "viewmanager.h"
#include "modemanager.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QPushButton>

#include <QInputDialog>
#include <QMessageBox>

#include <QGuiApplication>

CircuitListWidget::CircuitListWidget(ViewManager *mgr, CircuitListModel *model, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(mgr)
    , mModel(model)
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add Circuit"));
    remBut = new QPushButton(tr("Remove Circuit"));

    butLay->addWidget(addBut);
    butLay->addWidget(remBut);

    mView = new QTableView;
    lay->addWidget(mView);

    mView->setModel(mModel);

    // Use double click for opening view
    mView->setEditTriggers(QTableView::SelectedClicked);

    connect(mView, &QTableView::doubleClicked,
            this, &CircuitListWidget::onSceneDoubleClicked);

    connect(mModel->modeMgr(), &ModeManager::modeChanged,
            this, &CircuitListWidget::onFileModeChanged);

    connect(addBut, &QPushButton::clicked,
            this, &CircuitListWidget::addScene);
    connect(remBut, &QPushButton::clicked,
            this, &CircuitListWidget::removeCurrentScene);

    onFileModeChanged(mModel->modeMgr()->mode());
}

CircuitListModel *CircuitListWidget::model() const
{
    return mModel;
}

void CircuitListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void CircuitListWidget::addScene()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(this,
                                     tr("New Circuit Sheet"),
                                     first ?
                                         tr("Choose name:") :
                                         tr("Name is not available.\n"
                                            "Choose another name:"),
                                     QLineEdit::Normal,
                                     name);
        if(name.isEmpty())
            return;

        if(mModel->isNameAvailable(name))
            break;

        first = false;
    }

    mModel->addCircuitScene(name);
}

void CircuitListWidget::removeCurrentScene()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    CircuitScene *scene = mModel->sceneAtRow(idx.row());
    if(!scene)
        return;

    QString name = mModel->data(idx.siblingAtColumn(0),
                                Qt::DisplayRole).toString();

    int ret = QMessageBox::question(this,
                                    tr("Delete Scene?"),
                                    tr("Are you sure to delete <b>%1</b>?").arg(name));
    if(ret == QMessageBox::Yes)
    {
        mModel->removeSceneAtRow(idx.row());
    }
}

void CircuitListWidget::onSceneDoubleClicked(const QModelIndex &idx)
{
    // Open new view if shift is pressed, use existing otherwise
    const bool forceNew = QGuiApplication::keyboardModifiers()
            .testFlag(Qt::ShiftModifier);

    auto scene = mModel->sceneAtRow(idx.row());
    if(!scene)
        return;

    mViewMgr->addCircuitView(scene, forceNew);
}
