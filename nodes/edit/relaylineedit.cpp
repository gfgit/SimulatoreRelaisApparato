#include "relaylineedit.h"

#include <QCompleter>

#include "../../relaismodel.h"
#include "../../abstractrelais.h"

RelayLineEdit::RelayLineEdit(RelaisModel *m, QWidget *parent)
    : QLineEdit(parent)
    , mRelaisModel(m)
{
    QCompleter *c = new QCompleter;
    c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->setModel(mRelaisModel);
    setCompleter(c);

    connect(c, qOverload<const QModelIndex&>(&QCompleter::activated),
            this, [this](const QModelIndex& idx)
    {
        setRelais(mRelaisModel->relayAt(idx.row()));
    });
}

void RelayLineEdit::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;
    mRelais = newRelais;

    if(text() != mRelais->name())
        setText(mRelais->name());

    emit relayChanged(mRelais);
}
