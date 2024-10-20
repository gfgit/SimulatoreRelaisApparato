#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CircuitScene;
class AbstractNodeGraphItem;
class CableGraphItem;
class RelaisModel;
class NodeEditFactory;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    CircuitScene *scene() const;

private:
    void buildToolBar();

private slots:
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

private:
    Ui::MainWindow *ui;
    CircuitScene *mScene;
    RelaisModel *mRelaisModel;
    NodeEditFactory *mEditFactory;
};
#endif // MAINWINDOW_H
