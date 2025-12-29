/**
 * src/utils/genericleverutils.cpp
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

#include "genericleverutils.h"

#include <QJsonObject>
#include <QJsonArray>

#include <QCoreApplication>

class LeverPositionTypeNames
{
    Q_DECLARE_TR_FUNCTIONS(LeverPositionTypeNames)
public:
    static const char *texts[];
};

const char *LeverPositionTypeNames::texts[] = {QT_TRANSLATE_NOOP("LeverPositionTypeNames", "Exact"),
                                               QT_TRANSLATE_NOOP("LeverPositionTypeNames", "From/To")};

QString GenericLeverUtils::getTypeName(LeverPositionConditionType type)
{
    if (type >= LeverPositionConditionType::NTypes)
        return QString();
    return LeverPositionTypeNames::tr(LeverPositionTypeNames::texts[int(type)]);
}

LeverPositionConditionSet GenericLeverUtils::fromJSON(const QJsonObject &obj)
{
    LeverPositionConditionSet conditions;

    const QJsonArray arr = obj.value("array").toArray();
    conditions.reserve(arr.size());

    for(const QJsonValue& v : arr)
    {
        QJsonObject conditionObj = v.toObject();
        LeverPositionCondition item;

        item.positionFrom = conditionObj.value("a").toInt();
        item.positionTo = conditionObj.value("b").toInt();

        item.type = LeverPositionConditionType::Exact;
        if(conditionObj.value("type").toInt() != 0)
            item.type = LeverPositionConditionType::FromTo;

        item.specialContact = conditionObj.value("special").toBool();

        conditions.append(item);
    }

    return conditions;
}

QJsonObject GenericLeverUtils::toJSON(const LeverPositionConditionSet &conditions)
{
    QJsonArray arr;

    for(const LeverPositionCondition &item : conditions)
    {
        QJsonObject conditionObj;

        conditionObj["a"] = item.positionFrom;
        conditionObj["b"] = item.positionTo;
        conditionObj["type"] = int(item.type);
        conditionObj["special"] = item.specialContact;

        arr.append(conditionObj);
    }

    QJsonObject obj;
    obj["array"] = arr;

    return obj;
}
