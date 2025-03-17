/**
 * src/panels/view/panelwidget.h
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

#ifndef PanelWidget_H
#define PanelWidget_H

#include <QWidget>

class ViewManager;

class PanelScene;

class PanelView;
class QDoubleSpinBox;
class DoubleClickSlider;

class PanelItemFactory;

class PanelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanelWidget(ViewManager *mgr, QWidget *parent = nullptr);
    ~PanelWidget();

    PanelScene *scene() const;
    void setScene(PanelScene *newScene, bool updateName = true);

    PanelView *panelView() const;

    int uniqueNum() const;
    void setUniqueNum(int newUniqueNum);

    bool isStatusBarVisible() const;
    void setStatusBarVisible(bool val);

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

    void addNodeToCenter(PanelItemFactory *editFactory,
                         const QString& nodeType);

    void toggleStatusBar();

private:
    PanelScene *mScene = nullptr;

    PanelView *mCircuitView = nullptr;
    DoubleClickSlider *mZoomSlider = nullptr;
    QDoubleSpinBox *mZoomSpin = nullptr;

    QWidget *statusBar = nullptr;

    ViewManager *mViewMgr = nullptr;

    int mUniqueNum = 1;
};

#endif // PanelWidget_H
