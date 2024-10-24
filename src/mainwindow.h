/**
 * src/mainwindow.h
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
