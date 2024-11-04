#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QObject>
#include <QPointer>
#include <QHash>

#include "../enums/filemodes.h"

class MainWindow;

class CircuitWidget;
class CircuitListWidget;

class RelaisListWidget;

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
    ~ViewManager();

    CircuitWidget *activeCircuitView() const;

    CircuitWidget *addCircuitView(CircuitScene *scene,
                                  bool forceNew = false);

    void showCircuitSceneEdit(CircuitScene *scene);

    void closeAllEditDocks();
    void closeAllFileSpecificDocks();
    void closeAll();

public slots:
    void startEditNEwCableOnActiveView();
    void addNodeToActiveView(const QString& nodeType);
    void showCircuitListView();
    void showRelayListView();

private slots:
    void onCircuitViewDestroyed(QObject *obj);

    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

    void onFileModeChanged(FileMode mode, FileMode oldMode);

private:
    friend class CircuitWidget;
    void setActiveCircuit(CircuitWidget *w);

    void updateDockName(CircuitWidget *w);

    MainWindow *mainWin();

    int getUniqueNum(CircuitScene *scene, CircuitWidget *self) const;

private:
    // File specific views
    CircuitWidget *mActiveCircuitView = nullptr;

    QHash<CircuitWidget *, DockWidget *> mCircuitViews;

    // Edit views
    QHash<CircuitScene *, DockWidget *> mCircuitEdits;

    // General views
    QPointer<DockWidget> mCircuitListViewDock;
    QPointer<DockWidget> mRelaisListViewDock;
};

#endif // VIEWMANAGER_H
