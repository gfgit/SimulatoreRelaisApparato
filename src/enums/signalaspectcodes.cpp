/**
 * src/enums/signalaspectcodes.cpp
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

#include "signalaspectcodes.h"

#include <QCoreApplication>

class SignalAspectCodeTranslation
{
    Q_DECLARE_TR_FUNCTIONS(SignalAspectCodeTranslation)
};

static EnumDesc SignalAspectCode_desc =
{
    int(SignalAspectCode::CodeAbsent),
    int(SignalAspectCode::Code270),
    int(SignalAspectCode::CodeAbsent),
    "SignalAspectCodeTranslation",
    {
        QT_TRANSLATE_NOOP("SignalAspectCodeTranslation", "Code Absent"),
        QT_TRANSLATE_NOOP("SignalAspectCodeTranslation", "75"),
        QT_TRANSLATE_NOOP("SignalAspectCodeTranslation", "120"),
        QT_TRANSLATE_NOOP("SignalAspectCodeTranslation", "180"),
        QT_TRANSLATE_NOOP("SignalAspectCodeTranslation", "270")
    }
};

EnumDesc SignalAspectCode_getDesc()
{
    return SignalAspectCode_desc;
}
