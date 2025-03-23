#ifndef LIGHTRECTLIGHTSVIEW_H
#define LIGHTRECTLIGHTSVIEW_H

#include <QWidget>

class QPushButton;
class QTableView;

class LightRectLightsModel;
class LightRectItem;

class LightRectLightsView : public QWidget
{
    Q_OBJECT
public:
    explicit LightRectLightsView(QWidget *parent = nullptr);

    void loadFrom(LightRectItem *item);
    void saveTo(LightRectItem *item);

signals:
    void needsSave();

private slots:
    void onAdd();
    void onRemove();
    void onEdit();
    void editIndex(const QModelIndex& idx);
    void onMove(bool up);

private:
    LightRectLightsModel *mModel = nullptr;

    QTableView *mView;
    QPushButton *mAddLightBut;
    QPushButton *mRemLightBut;
    QPushButton *mEditLightBut;
    QPushButton *mUpLightBut;
    QPushButton *mDownLightBut;
};

#endif // LIGHTRECTLIGHTSVIEW_H
