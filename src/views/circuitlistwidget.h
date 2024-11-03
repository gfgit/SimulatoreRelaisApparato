#ifndef CIRCUITLISTWIDGET_H
#define CIRCUITLISTWIDGET_H

#include <QWidget>

#include "../enums/filemodes.h"

class CircuitListModel;
class QPushButton;
class QTableView;

class ViewManager;

class CircuitListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CircuitListWidget(ViewManager *mgr, CircuitListModel *model, QWidget *parent = nullptr);

    CircuitListModel *model() const;

private slots:
    void onFileModeChanged(FileMode mode);

    void addScene();
    void removeCurrentScene();

    void onSceneDoubleClicked(const QModelIndex& idx);

private:
    QTableView *mView;

    QPushButton *addBut;
    QPushButton *remBut;

    ViewManager *mViewMgr;
    CircuitListModel *mModel;
};

#endif // CIRCUITLISTWIDGET_H
