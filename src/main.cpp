/**
 * src/main.cpp
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

#include "mainwindow.h"

#include <QApplication>

#include <kddockwidgets-qt6/kddockwidgets/Config.h>
#include <kddockwidgets-qt6/kddockwidgets/MainWindow.h>

#include "info.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(AppCompany);
    QApplication::setApplicationName(AppProduct);
    QApplication::setApplicationDisplayName(AppDisplayName);
    QApplication::setApplicationVersion(AppVersion);

    qDebug() << QApplication::applicationDisplayName()
             << "Version:" << QApplication::applicationVersion() << "Built:" << AppBuildDate
             << "Website: " << AppProjectWebSite;
    qDebug() << "Qt:" << QT_VERSION_STR;
    qDebug() << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    auto& config = KDDockWidgets::Config::self();

    auto flags = config.flags();
    flags.setFlag(KDDockWidgets::Config::Flag_AllowReorderTabs);
    config.setFlags(flags);

    MainWindow w(QLatin1String("mainwindow1"));
    w.show();
    return app.exec();
}
