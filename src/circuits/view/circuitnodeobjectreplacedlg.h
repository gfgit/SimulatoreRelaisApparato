#ifndef CIRCUITNODEOBJECTREPLACEDLG_H
#define CIRCUITNODEOBJECTREPLACEDLG_H

#include <QDialog>
#include <QGroupBox>
#include <QVector>

class AbstractNodeGraphItem;

class SimulationObjectLineEdit;
class NodeGroupEditWidget;

class ViewManager;

class QVBoxLayout;

class CircuitNodeObjectReplaceDlg : public QDialog
{
    Q_OBJECT
public:
    explicit CircuitNodeObjectReplaceDlg(ViewManager *viewMgr,
                                         const QVector<AbstractNodeGraphItem *>& items,
                                         QWidget *parent = nullptr);

    void replaceName(const QString& oldStr, const QString &newStr);

    static void batchNodeEdit(const QVector<AbstractNodeGraphItem *> &items,
                              ViewManager *viewMgr,
                              QWidget *parent = nullptr);
protected:
    void done(int result) override;

private:
    void reloadGroups();
    void createGroups();

    void saveChanges();

private slots:
    void onReplaceName();

private:
    ViewManager *mViewMgr;
    QVBoxLayout *mGroupsLay;
    QVector<NodeGroupEditWidget *> mGroups;
    QVector<AbstractNodeGraphItem *> mItems;
};

class NodeGroupEditWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit NodeGroupEditWidget(
            const QVector<AbstractNodeGraphItem *>& items);

public:
    void createGroup(ViewManager *viewMgr);
    void reloadGroup(ViewManager *viewMgr);

    void saveChanges() const;

    void replaceName(ViewManager *viewMgr, const QString& oldStr, const QString &newStr);

private:
    QString mTypeName;

    struct PropertyValue
    {
        SimulationObjectLineEdit *mObjEdit;
        QVector<AbstractNodeGraphItem *> items;
        QString origObjectType;
        QString origObjectName;
    };

    struct PropertyEntry
    {
        QString propName;
        QString propPrettyName;
        QVector<PropertyValue> mValues;
    };

    QVector<PropertyEntry> mEntries;
    QVector<AbstractNodeGraphItem *> mItems;
};

#endif // CIRCUITNODEOBJECTREPLACEDLG_H
