#ifndef CIRCUITLISTWIDGET_H
#define CIRCUITLISTWIDGET_H

#include <QWidget>

#include "../enums/filemodes.h"

class CircuitListModel;
class CircuitScene;

class QLineEdit;
class QPushButton;
class QTableView;

class ViewManager;

class CircuitListWidget : public QWidget
{
    Q_OBJECT
public:
    CircuitListWidget(ViewManager *mgr, CircuitListModel *model, QWidget *parent = nullptr);

    CircuitListModel *model() const;

private slots:
    void onFileModeChanged(FileMode mode);

    void addScene();
    void removeCurrentScene();

    void onSceneDoubleClicked(const QModelIndex& idx);

    void showViewContextMenu(const QPoint& pos);

private:
    QTableView *mView;

    QPushButton *addBut;
    QPushButton *remBut;

    ViewManager *mViewMgr;
    CircuitListModel *mModel;
};

class CircuitSceneOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    CircuitSceneOptionsWidget(CircuitScene *scene, QWidget *parent = nullptr);

private slots:
    void setSceneName();
    void onNameTextEdited();
    void setSceneLongName();

private:
    void setNameValid(bool valid);

private:
    CircuitScene *mScene = nullptr;

    QLineEdit *mNameEdit;
    QLineEdit *mLongNameEdit;

    QPalette normalEditPalette;
};

#endif // CIRCUITLISTWIDGET_H
