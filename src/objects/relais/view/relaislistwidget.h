#ifndef RELAISLISTWIDGET_H
#define RELAISLISTWIDGET_H

#include <QWidget>

#include "../../../enums/filemodes.h"

class RelaisModel;
class QSortFilterProxyModel;

class QPushButton;
class QTableView;

class ViewManager;

class RelaisListWidget : public QWidget
{
    Q_OBJECT
public:
    RelaisListWidget(ViewManager *mgr, RelaisModel *model, QWidget *parent = nullptr);

    RelaisModel *model() const;

private slots:
    void onFileModeChanged(FileMode mode);

    void addRelay();
    void removeCurrentRelay();
private:
    QTableView *mView;

    QPushButton *addBut;
    QPushButton *remBut;

    ViewManager *mViewMgr;
    RelaisModel *mModel;

    QSortFilterProxyModel *mProxyModel;
};

#endif // RELAISLISTWIDGET_H
