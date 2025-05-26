#include "replicaslistwidget.h"

#include "../../views/viewmanager.h"
#include "../../views/modemanager.h"

#include "../remotemanager.h"
#include "../replicaobjectmanager.h"

#include "../replicasmodel.h"
#include "replicasdelegate.h"

#include "../../objects/simulationobjectlineedit.h"
#include "../../objects/simulationobjectfactory.h"

#include <QTableView>
#include <QPushButton>

#include <QPointer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <QBoxLayout>
#include <QFormLayout>

ReplicasListWidget::ReplicasListWidget(ViewManager *viewMgr, QWidget *parent)
    : QWidget{parent}
    , mViewMgr(viewMgr)
{
    ReplicaObjectManager *replicaMgr = mViewMgr->modeMgr()->getRemoteManager()->replicaMgr();
    mModel = replicaMgr->replicasModel();

    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *butLay = new QHBoxLayout;
    lay->addLayout(butLay);

    addBut = new QPushButton(tr("Add"));
    addBut->setToolTip(tr("Add Replica"));
    butLay->addWidget(addBut);

    remBut = new QPushButton(tr("Remove"));
    remBut->setToolTip(tr("Remove Replica"));
    butLay->addWidget(remBut);

    mView = new QTableView;
    mView->setModel(mModel);
    mView->setItemDelegate(new ReplicasDelegate(replicaMgr->remoteMgr()->remoteSessionsModel(), this));
    lay->addWidget(mView);

    connect(addBut, &QPushButton::clicked,
            this, &ReplicasListWidget::addReplica);
    connect(remBut, &QPushButton::clicked,
            this, &ReplicasListWidget::removeReplica);

    connect(mViewMgr->modeMgr(), &ModeManager::modeChanged,
            this, &ReplicasListWidget::onFileModeChanged);

    onFileModeChanged(mViewMgr->modeMgr()->mode());
}

void ReplicasListWidget::onFileModeChanged(FileMode mode)
{
    const bool canEdit = mode == FileMode::Editing;
    addBut->setEnabled(canEdit);
    addBut->setVisible(canEdit);
    remBut->setEnabled(canEdit);
    remBut->setVisible(canEdit);
}

void ReplicasListWidget::addReplica()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    ReplicaObjectManager *replicaMgr = mViewMgr->modeMgr()->getRemoteManager()->replicaMgr();
    AbstractSimulationObject *replicaObj = nullptr;

    const auto replicaTypes = mViewMgr->modeMgr()->objectFactory()->replicaTypes();
    if(replicaTypes.isEmpty())
        return;

    QPointer<QDialog> dlg = new QDialog(this);
    QFormLayout *lay = new QFormLayout(dlg);

    SimulationObjectLineEdit *objEdit = new SimulationObjectLineEdit(mViewMgr,
                                                                     replicaTypes);
    lay->addRow(tr("Replica:"), objEdit);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lay->addRow(box);

    connect(box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    while(dlg)
    {
        if(dlg->exec() != QDialog::Accepted)
            break;

        replicaObj = objEdit->getObject();
        if(!replicaObj)
            continue;

        if(!replicaMgr->addReplicaObject(replicaObj))
        {
            objEdit->setObject(nullptr);
            continue;
        }

        break; // Success
    }

    delete dlg;
}

void ReplicasListWidget::removeReplica()
{
    if(mViewMgr->modeMgr()->mode() != FileMode::Editing)
        return;

    QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    mModel->removeAt(idx.row());
}
