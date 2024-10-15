#ifndef ABSTRACTNODEGRAPHITEM_H
#define ABSTRACTNODEGRAPHITEM_H

#include <QGraphicsObject>

class AbstractCircuitNode;

class AbstractNodeGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    AbstractNodeGraphItem(AbstractCircuitNode *node_);

    inline AbstractCircuitNode *getAbstractNode() const
    {
        return mAbstractNode;
    }

protected slots:
    void triggerUpdate();
    virtual void updateName();

private:
    AbstractCircuitNode *mAbstractNode;
};

#endif // ABSTRACTNODEGRAPHITEM_H
