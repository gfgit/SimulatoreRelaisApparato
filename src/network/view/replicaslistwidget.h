#ifndef REPLICAS_LIST_WIDGET_H
#define REPLICAS_LIST_WIDGET_H

#include <QWidget>

#include "../../enums/filemodes.h"

class QPushButton;
class QTableView;

class ViewManager;
class ReplicasModel;

class ReplicasListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReplicasListWidget(ViewManager *viewMgr,
                                QWidget *parent = nullptr);

private slots:
    void onFileModeChanged(FileMode mode);

    void addReplica();
    void removeReplica();

private:
    ViewManager *mViewMgr;

    QTableView *mView;
    QPushButton *addBut;
    QPushButton *remBut;

    ReplicasModel *mModel;
};

#endif // REPLICAS_LIST_WIDGET_H
