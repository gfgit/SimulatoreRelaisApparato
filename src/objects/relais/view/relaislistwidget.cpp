#include "relaislistwidget.h"

#include "../model/relaismodel.h"
#include "../model/abstractrelais.h"

#include "../../../views/viewmanager.h"
#include "../../../views/modemanager.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QPushButton>

#include <QInputDialog>
#include <QMessageBox>

#include <QSortFilterProxyModel>

RelaisListWidget::RelaisListWidget(ViewManager *mgr, RelaisModel *model, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(mgr)
    , mModel(model)
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add Relay"));
    remBut = new QPushButton(tr("Remove Relay"));

    butLay->addWidget(addBut);
    butLay->addWidget(remBut);

    mView = new QTableView;
    lay->addWidget(mView);

    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSourceModel(mModel);
    mProxyModel->setSortRole(Qt::DisplayRole);
    mProxyModel->sort(0);

    mView->setModel(mProxyModel);

    connect(mModel->modeMgr(), &ModeManager::modeChanged,
            this, &RelaisListWidget::onFileModeChanged);

    connect(addBut, &QPushButton::clicked,
            this, &RelaisListWidget::addRelay);
    connect(remBut, &QPushButton::clicked,
            this, &RelaisListWidget::removeCurrentRelay);

    onFileModeChanged(mModel->modeMgr()->mode());
}

RelaisModel *RelaisListWidget::model() const
{
    return mModel;
}

void RelaisListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void RelaisListWidget::addRelay()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QString name;

    bool first = true;
    while(true)
    {
        name = QInputDialog::getText(this,
                                     tr("New Relay"),
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

    AbstractRelais *r = new AbstractRelais(mModel);
    r->setName(name);
    mModel->addRelay(r);
}

void RelaisListWidget::removeCurrentRelay()
{
    if(mModel->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    idx = mProxyModel->mapToSource(idx);
    if(!idx.isValid())
        return;

    AbstractRelais *r = mModel->relayAt(idx.row());
    if(!r)
        return;

    int ret = QMessageBox::question(this,
                                    tr("Delete Relais?"),
                                    tr("Are you sure to delete <b>%1</b>?")
                                    .arg(r->name()));
    if(ret == QMessageBox::Yes)
    {
        mModel->removeRelay(r);
    }
}
