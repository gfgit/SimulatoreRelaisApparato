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
    void updateRecentFileActions();

    void loadFile(const QString &fileName);

    void locateAppSettings();

private slots:
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

    void onOpen();
    void onOpenRecent();
    void onSave();

private:
    Ui::MainWindow *ui;
    CircuitScene *mScene;
    RelaisModel *mRelaisModel;
    NodeEditFactory *mEditFactory;

    enum
    {
        MaxRecentFiles = 10
    };
    QAction *recentFileActs[MaxRecentFiles];
    QString settingsFile;
};
#endif // MAINWINDOW_H
