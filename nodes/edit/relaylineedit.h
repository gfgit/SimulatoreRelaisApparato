#ifndef RELAYLINEEDIT_H
#define RELAYLINEEDIT_H

#include <QLineEdit>

class RelaisModel;
class AbstractRelais;

class RelayLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    RelayLineEdit(RelaisModel *m, QWidget *parent = nullptr);

    AbstractRelais *relais() const
    {
        return mRelais;
    }

public slots:
    void setRelais(AbstractRelais *newRelais);

signals:
    void relayChanged(AbstractRelais *r);

private:
    RelaisModel *mRelaisModel = nullptr;
    AbstractRelais *mRelais = nullptr;
};







#endif // RELAYLINEEDIT_H
