#include "circuitlistwidget.h"

#include "circuitlistmodel.h"
#include "../graph/circuitscene.h"

#include "viewmanager.h"
#include "modemanager.h"

#include <QVBoxLayout>
#include <QFormLayout>

#include <QTableView>
#include <QPushButton>
#include <QLineEdit>

#include <QInputDialog>
#include <QMessageBox>

#include <QGuiApplication>

#include <QMenu>
#include <QAction>

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
    mView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Edit through dedicate options widget
    mView->setEditTriggers(QTableView::NoEditTriggers);

    connect(mView, &QTableView::doubleClicked,
            this, &CircuitListWidget::onSceneDoubleClicked);

    connect(mModel->modeMgr(), &ModeManager::modeChanged,
            this, &CircuitListWidget::onFileModeChanged);

    connect(addBut, &QPushButton::clicked,
            this, &CircuitListWidget::addScene);
    connect(remBut, &QPushButton::clicked,
            this, &CircuitListWidget::removeCurrentScene);

    connect(mView, &QTableView::customContextMenuRequested,
            this, &CircuitListWidget::showViewContextMenu);

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

void CircuitListWidget::showViewContextMenu(const QPoint &pos)
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QPointer<QMenu> menu = new QMenu(this);

    QModelIndex idx = mView->indexAt(pos);
    CircuitScene *scene = mModel->sceneAtRow(idx.row());
    if(!scene)
        return;

    QAction *actionEdit = menu->addAction(tr("Edit"));
    QAction *ret = menu->exec(mView->viewport()->mapToGlobal(pos));
    if(ret == actionEdit)
        mViewMgr->showCircuitSceneEdit(scene);
}

CircuitSceneOptionsWidget::CircuitSceneOptionsWidget(CircuitScene *scene, QWidget *parent)
    : QWidget{parent}
    , mScene(scene)
{
    QFormLayout *lay = new QFormLayout(this);

    mNameEdit = new QLineEdit;
    mNameEdit->setPlaceholderText(tr("Name"));
    lay->addRow(tr("Name:"), mNameEdit);

    normalEditPalette = mNameEdit->palette();

    mLongNameEdit = new QLineEdit;
    mLongNameEdit->setPlaceholderText(tr("Long Name"));
    lay->addRow(tr("Long Name:"), mLongNameEdit);

    mNameEdit->setText(mScene->circuitSheetName());
    mLongNameEdit->setText(mScene->circuitSheetLongName());

    connect(mNameEdit, &QLineEdit::editingFinished,
            this, &CircuitSceneOptionsWidget::setSceneName);
    connect(mNameEdit, &QLineEdit::textEdited,
            this, &CircuitSceneOptionsWidget::onNameTextEdited);

    connect(mLongNameEdit, &QLineEdit::editingFinished,
            this, &CircuitSceneOptionsWidget::setSceneLongName);
}

void CircuitSceneOptionsWidget::setSceneName()
{
    QString newName = mNameEdit->text().trimmed();
    if(!newName.isEmpty() && mScene->setCircuitSheetName(newName))
        return;

    // Name is not valid, go back to old name
    mNameEdit->setText(mScene->circuitSheetName());
    setNameValid(true);
}

void CircuitSceneOptionsWidget::onNameTextEdited()
{
    QString newName = mNameEdit->text().trimmed();

    bool valid = true;
    if(newName != mScene->circuitSheetName())
        valid = mScene->circuitsModel()->isNameAvailable(newName);

    setNameValid(valid);
}

void CircuitSceneOptionsWidget::setSceneLongName()
{
    QString newLongName = mLongNameEdit->text().trimmed();
    mScene->setCircuitSheetLongName(newLongName);
}

void CircuitSceneOptionsWidget::setNameValid(bool valid)
{
    if(valid)
    {
        mNameEdit->setPalette(normalEditPalette);
        mNameEdit->setToolTip(QString());
    }
    else
    {
        // Red text
        QPalette p = mNameEdit->palette();
        p.setColor(QPalette::Text, Qt::red);
        mNameEdit->setPalette(p);

        mNameEdit->setToolTip(tr("Name already exists"));
    }
}
