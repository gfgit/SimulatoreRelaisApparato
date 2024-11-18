/**
 * src/objects/simple_activable/electromagnet.h
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

#ifndef ELECTROMAGNET_OBJECT_H
#define ELECTROMAGNET_OBJECT_H

#include "abstractsimpleactivableobject.h"

class ElectroMagnetObject : public AbstractSimpleActivableObject
{
    Q_OBJECT
public:
    explicit ElectroMagnetObject(AbstractSimulationObjectModel *m);

    static constexpr QLatin1String Type = QLatin1String("electromagnet");
    QString getType() const override;
};

#endif // ELECTROMAGNET_OBJECT_H