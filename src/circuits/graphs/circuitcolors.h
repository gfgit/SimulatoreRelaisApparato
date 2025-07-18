/**
 * src/circuits/graphs/circuitcolors.h
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

#ifndef CIRCUITCOLORS_H
#define CIRCUITCOLORS_H

#include <QRgb>

namespace CircuitColors
{

// Standard
constexpr QRgb None = qRgb(0, 0, 0); // Black
constexpr QRgb Closed = qRgb(255, 0, 0); // Full red
constexpr QRgb Open = qRgb(120, 210, 255); // Light blue

// Resistor
constexpr QRgb ClosedResistor = qRgb(255, 120, 120); // Light red
constexpr QRgb OpenResistor = qRgb(170, 230, 255); // Very Light blue

// Code
constexpr QRgb Code75 = qRgb(0, 255, 0); // green
constexpr QRgb Code120 = qRgb(0, 120, 0); // dark green
constexpr QRgb CodeInvalid = qRgb(255, 255, 0); // yellow

}

#endif // CIRCUITCOLORS_H
