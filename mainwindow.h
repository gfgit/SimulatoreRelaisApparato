#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CircuitScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    CircuitScene *scene() const;

private:
    void buildToolBar();

private:
    Ui::MainWindow *ui;
    CircuitScene *mScene;
};
#endif // MAINWINDOW_H
