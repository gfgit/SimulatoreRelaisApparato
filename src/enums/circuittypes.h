/**
 * src/enums/circuittypes.h
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

#ifndef CIRCUITTYPES_H
#define CIRCUITTYPES_H

#include <QtTypes>

enum class CircuitType
{
    Open = 0,
    Closed = 1
};

enum class AnyCircuitType
{
    Open = 0,
    Closed = 1,
    None = 2
};

enum class CircuitPole
{
    First = 0,
    Second = 1
};

enum class CircuitFlags : quint8
{
    None = 0,
    Resistor    = 0b00000001,
    Code75      = 0b00000010,
    Code120     = 0b00000100,
    Code180     = 0b00001000,
    Code270     = 0b00010000,
    CodeInvalid = 0b00100000,
    CodeMask    = 0b00111110,
    NonCodeMask = 0b11000001,

    FlagsMask   = 0b00111111,
    PoleMask    = 0b11000000,
    FromPole    = 0b01000000,
    ToPole      = 0b10000000
};

constexpr CircuitFlags operator |(CircuitFlags a, CircuitFlags b)
{
    return static_cast<CircuitFlags>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr CircuitFlags operator &(CircuitFlags a, CircuitFlags b)
{
    return static_cast<CircuitFlags>(static_cast<int>(a) & static_cast<int>(b));
}

constexpr CircuitFlags onlyFlags(CircuitFlags flags)
{
    return flags & CircuitFlags::FlagsMask;
}

constexpr CircuitPole fromPole_(CircuitFlags flags)
{
    return ((flags & CircuitFlags::FromPole) == CircuitFlags::FromPole)
            ? CircuitPole::Second : CircuitPole::First;
}

constexpr CircuitPole toPole_(CircuitFlags flags)
{
    return ((flags & CircuitFlags::ToPole) == CircuitFlags::ToPole)
            ? CircuitPole::Second : CircuitPole::First;
}

constexpr CircuitFlags withPole(CircuitFlags flags, CircuitPole from, CircuitPole to)
{
    flags = onlyFlags(flags);
    flags = flags | static_cast<CircuitFlags>(int(from) << 6);
    flags = flags | static_cast<CircuitFlags>(int(to) << 7);
    return flags;
}

constexpr CircuitFlags applyPole(CircuitFlags flags, CircuitFlags poles)
{
    return (flags & CircuitFlags::FlagsMask) | (poles & CircuitFlags::PoleMask);
}

constexpr CircuitFlags getCode(CircuitFlags flags)
{
    CircuitFlags code = flags & CircuitFlags::CodeMask;
    switch (code)
    {
    case CircuitFlags::Code75:
    case CircuitFlags::Code120:
    case CircuitFlags::Code180:
    case CircuitFlags::Code270:
        return code;
    case CircuitFlags::None:
        return CircuitFlags::None;
    default:
        return CircuitFlags::CodeInvalid;
    }
}

constexpr CircuitFlags withCode(CircuitFlags flags, CircuitFlags code)
{
    const CircuitFlags withoutCode = flags & CircuitFlags::NonCodeMask;
    return withoutCode | (code & CircuitFlags::CodeMask);
}

constexpr CircuitType toType_(CircuitFlags flags)
{
    return ((flags & CircuitFlags::ToPole) == CircuitFlags::ToPole)
            ? CircuitType::Closed : CircuitType::Open;
}

constexpr CircuitFlags withType(CircuitFlags flags, CircuitType type)
{
    flags = onlyFlags(flags);
    flags = flags | static_cast<CircuitFlags>(int(type) << 7);
    return flags;
}

constexpr bool hasResistor(CircuitFlags flags)
{
    return ((flags & CircuitFlags::Resistor) == CircuitFlags::Resistor);
}

#endif // CIRCUITTYPES_H
