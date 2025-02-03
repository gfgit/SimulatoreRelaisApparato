/**
 * src/views/layoutloader.h
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#ifndef LAYOUT_LOADER_H
#define LAYOUT_LOADER_H

namespace KDDockWidgets {
namespace Core {
class DockWidget;
}
namespace QtWidgets {
class DockWidget;
}
}

class QString;
class QByteArray;

class LayoutLoader
{
public:
    static void registerLoader();


    static void loadLayout(const QByteArray& data);

private:
    static KDDockWidgets::QtWidgets::DockWidget *createDockWidget(const QString &name);
    static KDDockWidgets::Core::DockWidget *dockWidgetFactoryFunc(const QString &name);
    static void setDeleteOnClose(bool value);
};

#endif // LAYOUT_LOADER_H
