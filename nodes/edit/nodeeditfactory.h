#ifndef NODEEDITFACTORY_H
#define NODEEDITFACTORY_H

#include <QObject>
#include <QString>
#include <QVector>

class CableGraphItem;
class AbstractNodeGraphItem;
class AbstractCircuitNode;
class CircuitScene;

class QWidget;

class NodeEditFactory : public QObject
{
    Q_OBJECT
public:
    NodeEditFactory(QObject *parent);

    typedef AbstractNodeGraphItem *(*CreateFunc)(CircuitScene *parent);
    typedef QWidget*(*EditFunc)(AbstractNodeGraphItem *item);

    struct FactoryItem
    {
        QString nodeType;
        QString prettyName;
        CreateFunc create = nullptr;
        EditFunc edit = nullptr;
        bool needsName = false;
    };

    QStringList getRegisteredTypes() const;

    AbstractNodeGraphItem *createItem(const QString& nodeType, CircuitScene *parent);
    void editItem(QWidget *parent, AbstractNodeGraphItem *item);
    void editCable(QWidget *parent, CableGraphItem *item);

    QString prettyName(const QString& nodeType) const;
    bool needsName(const QString &nodeType) const;

    void registerFactory(const FactoryItem& factory);

private:
    const FactoryItem *getItemForType(const QString& nodeType) const;

private:
    QVector<FactoryItem> mItems;
};

#endif // NODEEDITFACTORY_H
