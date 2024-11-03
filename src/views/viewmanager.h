#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QObject>
#include <QPointer>
#include <QHash>

class MainWindow;
class CircuitWidget;
class CircuitListWidget;

class CircuitScene;
class AbstractNodeGraphItem;
class CableGraphItem;

namespace KDDockWidgets::QtWidgets
{
class DockWidget;
}

class ViewManager : public QObject
{
    Q_OBJECT
public:
    typedef KDDockWidgets::QtWidgets::DockWidget DockWidget;

    explicit ViewManager(MainWindow *parent);

    CircuitWidget *activeCircuitView() const;

    CircuitWidget *addCircuitView(CircuitScene *scene,
                                  bool forceNew = false);


public slots:
    void startEditNEwCableOnActiveView();
    void addNodeToActiveView(const QString& nodeType);
    void showCircuitListView();

private slots:
    void onCircuitViewDestroyed(QObject *obj);

    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

private:
    friend class CircuitWidget;
    void setActiveCircuit(CircuitWidget *w);

    void updateDockName(CircuitWidget *w);

    MainWindow *mainWin();

    int getUniqueNum(CircuitScene *scene) const;

private:
    CircuitWidget *mActiveCircuitView = nullptr;

    QHash<CircuitWidget *, DockWidget *> mCircuitViews;

    QPointer<DockWidget> mCircuitListViewDock;
};

#endif // VIEWMANAGER_H
