#include "lightrectlightsview.h"

#include "lightrectlightsmodel.h"

#include "../../objects/simulationobjectlineedit.h"
#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../views/viewmanager.h"

#include "../../utils/colorselectionwidget.h"

#include "../graphs/lightrectitem.h"

#include <QTableView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPointer>

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

class LightEntryDialog : public QDialog
{
public:
    LightEntryDialog(QWidget *parent) : QDialog(parent)
    {
        QFormLayout *lay = new QFormLayout(this);

        lightEdit = new SimulationObjectLineEdit(ViewManager::self(),
                                                 {LightBulbObject::Type});
        lay->addRow(tr("Light:"), lightEdit);

        colorEdit = new ColorSelectionWidget;
        lay->addRow(tr("Color:"), colorEdit);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                     Qt::Horizontal);
        connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
        lay->addRow(box);
    }

    void setEntry(LightBulbObject *obj, const QColor &color)
    {
        lightEdit->setObject(obj);
        colorEdit->setColor(color);
    }

    QColor getColor() const
    {
        return colorEdit->color();
    }

    LightBulbObject *getLight() const
    {
        return static_cast<LightBulbObject *>(lightEdit->getObject());
    }

private:
    SimulationObjectLineEdit *lightEdit;
    ColorSelectionWidget *colorEdit;
};

LightRectLightsView::LightRectLightsView(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    mModel = new LightRectLightsModel(this);
    connect(mModel, &LightRectLightsModel::needsSave,
            this, &LightRectLightsView::needsSave);

    mView = new QTableView;
    mView->setSelectionBehavior(QTableView::SelectRows);
    mView->setEditTriggers(QTableView::NoEditTriggers);
    mView->setModel(mModel);
    lay->addWidget(mView);

    QVBoxLayout *butLay = new QVBoxLayout;
    lay->addLayout(butLay);

    mAddLightBut = new QPushButton(tr("Add Light"));
    mRemLightBut = new QPushButton(tr("Remove Light"));
    mEditLightBut = new QPushButton(tr("Edit Light"));
    mUpLightBut = new QPushButton(tr("Move Up"));
    mDownLightBut = new QPushButton(tr("Move Down"));

    butLay->addWidget(mAddLightBut);
    butLay->addWidget(mRemLightBut);
    butLay->addWidget(mEditLightBut);
    butLay->addWidget(mUpLightBut);
    butLay->addWidget(mDownLightBut);

    connect(mAddLightBut, &QPushButton::clicked,
            this, &LightRectLightsView::onAdd);
    connect(mRemLightBut, &QPushButton::clicked,
            this, &LightRectLightsView::onRemove);
    connect(mEditLightBut, &QPushButton::clicked,
            this, &LightRectLightsView::onEdit);
    connect(mUpLightBut, &QPushButton::clicked,
            this, [this]()
    {
        onMove(true);
    });
    connect(mDownLightBut, &QPushButton::clicked,
            this, [this]()
    {
        onMove(false);
    });
    connect(mView, &QTableView::activated,
            this, &LightRectLightsView::editIndex);
}

void LightRectLightsView::loadFrom(LightRectItem *item)
{
    QVector<LightRectLightsModel::LightEntry> entries;
    entries.reserve(item->lights().size());
    for(const auto& entry : item->lights())
        entries.append({entry.light, entry.color});
    mModel->setItems(entries);
}

void LightRectLightsView::saveTo(LightRectItem *item)
{
    if(!mModel->isChanged())
        return;

    QVector<LightRectItem::LightEntry> entries;
    entries.reserve(mModel->items().size());
    for(const auto& entry : mModel->items())
        entries.append({entry.light, entry.color});
    item->setLights(entries);

    mModel->resetChanged();
}

void LightRectLightsView::onAdd()
{
    QPointer<LightEntryDialog> dlg = new LightEntryDialog(this);

    // Default to red and then yellow for following lights
    dlg->setEntry(nullptr, mModel->rowCount() == 0 ? Qt::red : Qt::yellow);

    if(dlg->exec() == QDialog::Accepted && dlg && dlg->getLight())
    {
        LightRectLightsModel::LightEntry entry{dlg->getLight(), dlg->getColor()};

        // Insert after current row or as last row
        const QModelIndex idx = mView->currentIndex();
        const int row = idx.isValid() ? idx.row() + 1 : mModel->rowCount();
        mModel->addEntryAt(row, entry);
    }

    delete dlg;
}

void LightRectLightsView::onRemove()
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    mModel->removeEntryAt(idx.row());
}

void LightRectLightsView::onEdit()
{
    const QModelIndex idx = mView->currentIndex();
    editIndex(idx);
}

void LightRectLightsView::editIndex(const QModelIndex &idx)
{
    if(!idx.isValid())
        return;

    QPointer<LightEntryDialog> dlg = new LightEntryDialog(this);
    LightRectLightsModel::LightEntry entry = mModel->getEntryAt(idx.row());
    dlg->setEntry(entry.light, entry.color);

    if(dlg->exec() == QDialog::Accepted && dlg && dlg->getLight())
    {
        entry.light = dlg->getLight();
        entry.color = dlg->getColor();
        mModel->setEntryAt(idx.row(), entry);
    }

    delete dlg;
}

void LightRectLightsView::onMove(bool up)
{
    const QModelIndex idx = mView->currentIndex();
    if(!idx.isValid())
        return;

    mModel->moveRow(idx.row(), up);
}
