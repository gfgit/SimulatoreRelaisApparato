#ifndef RELAISPOWERGRAPHITEM_H
#define RELAISPOWERGRAPHITEM_H


#include "abstractnodegraphitem.h"

class RelaisPowerNode;
class AbstractRelais;

class RelaisPowerGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef RelaisPowerNode Node;

    RelaisPowerGraphItem(RelaisPowerNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void getConnectors(std::vector<Connector>& connectors) const final;

    RelaisPowerNode *node() const;

private slots:
    void updateRelay();

protected slots:
    void updateName() override;

private:
    AbstractRelais *mRelay = nullptr;
};

#endif // RELAISPOWERGRAPHITEM_H
