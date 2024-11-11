/**
 * src/enums/filemodes.h
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

#ifndef FILEMODES_H
#define FILEMODES_H

enum class FileMode
{
    Simulation = 0,
    Editing,
    LoadingFile
};

enum class EditingSubMode
{
    // In default mode you can add nodes to circuit,
    // you can rotate or delete single nodes.
    // You can also trigger other modes, like by adding a cable
    Default = 0,

    // A single node is being moved by dragging it with mouse
    // When mouse is released it will be fixed in it's position
    SingleItemMove,

    // In cable editing mode you can edit currently selected cable path
    // Node move and rotation is inibited to prevent rotating by mistake.
    CableEditing,

    // In item selection you can select multiple nodes and cables
    // You can move them togheter, or delete them
    // You can copy/paste them
    ItemSelection
};

#endif // FILEMODES_H
