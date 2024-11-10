/**
 * src/circuits/view/circuitwidget.h
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
    void focusInEvent(QFocusEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;

private:
    friend class ViewManager;

    void addNodeToCenter(NodeEditFactory *editFactory,
                         const QString& nodeType);

    void toggleStatusBar();

private:
    CircuitScene *mScene = nullptr;

    ZoomGraphView *mCircuitView = nullptr;
    DoubleClickSlider *mZoomSlider = nullptr;
    QDoubleSpinBox *mZoomSpin = nullptr;

    QWidget *statusBar = nullptr;

    ViewManager *mViewMgr = nullptr;

    int mUniqueNum = 1;
};

#endif // CIRCUITWIDGET_H
