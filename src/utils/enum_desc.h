/**
 * src/utils/enum_desc.h
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

#ifndef ENUM_DESC_H
#define ENUM_DESC_H

#include <QByteArray>
#include <QByteArrayList>

struct EnumDesc
{
    int minValue = 0;
    int maxValue = 0;
    int defaultValue = 0;

    QByteArray translationContext;
    QByteArrayList valueNamesFromMin;

    inline QByteArray untranslatedName(int value) const
    {
        if(value < minValue || value > maxValue)
            return QByteArray();

        return valueNamesFromMin.at(value - minValue);
    }

    QString name(int value) const;
};

#endif // ENUM_DESC_H
