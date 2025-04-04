/**
 * src/objects/simulationobjectcopyhelper.h
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

#ifndef SIMULATIONOBJECTCOPYHELPER_H
#define SIMULATIONOBJECTCOPYHELPER_H

#include <QHash>
#include <QStringList>

class ModeManager;
class AbstractSimulationObject;

class QJsonObject;

class SimulationObjectCopyHelper
{
public:
    static constexpr const QLatin1String CircuitMimeType = QLatin1String("application/x-simulatore-rele-circuits");

    static QJsonObject copyObjects(ModeManager *modeMgr,
                                   const QHash<QString, QStringList> &objMap);

    static QJsonObject copyObjects(ModeManager *modeMgr,
                                   const QVector<AbstractSimulationObject *> &objToCopy);

    static void pasteObjects(ModeManager *modeMgr,
                             const QJsonObject& objectPool);

    static void copyToClipboard(const QJsonObject& data);

    static bool getPasteDataFromClipboard(QJsonObject &data);
};

#endif // SIMULATIONOBJECTCOPYHELPER_H
