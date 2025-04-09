/**
 * src/enums/signalaspectcodes.h
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

#ifndef SIGNALASPECTCODES_H
#define SIGNALASPECTCODES_H

#include "../utils/enum_desc.h"

enum SignalAspectCode : uint8_t
{
    CodeAbsent = 0,
    Code75,
    Code120,
    Code180,
    Code270
};

inline constexpr int codeToNumber(SignalAspectCode code)
{
    switch (code)
    {
    case SignalAspectCode::Code75:
        return 75;
        break;
    case SignalAspectCode::Code120:
        return 120;
        break;
    case SignalAspectCode::Code180:
        return 180;
        break;
    case SignalAspectCode::Code270:
        return 270;
        break;
    default:
        break;
    }

    return 0;
}

inline constexpr SignalAspectCode codeFromNumber(int code)
{
    switch (code)
    {
    case 75:
        return SignalAspectCode::Code75;
        break;
    case 120:
        return SignalAspectCode::Code120;
        break;
    case 180:
        return SignalAspectCode::Code180;
        break;
    case 270:
        return SignalAspectCode::Code270;
        break;
    default:
        break;
    }

    return SignalAspectCode::CodeAbsent;
}

extern EnumDesc SignalAspectCode_getDesc();

#endif // SIGNALASPECTCODES_H
