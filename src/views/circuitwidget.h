#ifndef CIRCUITWIDGET_H
#define CIRCUITWIDGET_H

#include <QWidget>

class ViewManager;

class CircuitScene;

class ZoomGraphView;
class QDoubleSpinBox;
class DoubleClickSlider;

class NodeEditFactory;

class CircuitWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CircuitWidget(ViewManager *mgr, QWidget *parent = nullptr);
    ~CircuitWidget();

    CircuitScene *scene() const;
    void setScene(CircuitScene *newScene, bool updateName = true);

    ZoomGraphView *circuitView() const;

    int uniqueNum() const;
    void setUniqueNum(int newUniqueNum);

private slots:
    void onZoomChanged(double val);
    void onZoomSliderChanged(int val);
    void onZoomSpinChanged(double val);
    void resetZoom();

    void onSceneNameChanged();
    void onSceneDestroyed();

protected:
    bool eventFilter(QObject *watched, QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private:
    friend class ViewManager;

    void addNodeToCenter(NodeEditFactory *editFactory,
                         const QString& nodeType);

private:
    CircuitScene *mScene = nullptr;

    ZoomGraphView *mCircuitView;
    DoubleClickSlider *mZoomSlider;
    QDoubleSpinBox *mZoomSpin;

    QWidget *statusBar;

    ViewManager *mViewMgr;

    int mUniqueNum = 1;
};

#endif // CIRCUITWIDGET_H
