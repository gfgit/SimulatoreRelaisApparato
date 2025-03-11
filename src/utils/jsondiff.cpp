/**
 * src/utils/jsondiff.cpp
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

#include "jsondiff.h"

#include <QJsonObject>

static constexpr QLatin1String NameKey = QLatin1String("name");

void JSONDiff::checkDifferences(const QJsonObject &origSettings, QJsonObject &newSettings)
{
    QJsonObject diffObj;

    for(const QString& key : newSettings.keys())
    {
        const QJsonValue oldVal = origSettings[key];
        const QJsonValueRef newVal = newSettings[key];

        if(oldVal.isObject() != newVal.isObject())
        {
            diffObj[key] = newVal;
            continue;
        }

        if(oldVal.isObject())
        {
            // Check sub object
            QJsonObject subObject = newVal.toObject();
            checkDifferences(oldVal.toObject(),
                             subObject);

            // Overwrite with only changed keys
            diffObj[key] = subObject;
            continue;
        }

        if(oldVal.toVariant() != newVal.toVariant())
        {
            diffObj[key] = newVal;
            continue;
        }
    }

    newSettings = diffObj;
}

QStringList JSONDiff::checkDifferencesTopLevel(const QJsonObject &origSettings, QJsonObject &newSettings, QString &namePrefix, QString &nameSuffix)
{
    QStringList modifiedKeys;


    for(const QString& key : newSettings.keys())
    {
        if(key == NameKey)
        {
            const QString oldName = origSettings[NameKey].toString();
            const QString newName = newSettings[NameKey].toString();

            int idx = newName.indexOf(oldName);
            if(idx >= 0)
            {
                if(idx > 0)
                    namePrefix = newName.mid(0, idx);

                if((idx + oldName.length()) < newName.size())
                    nameSuffix = newName.mid(idx + oldName.length());

                modifiedKeys.append(NameKey);
            }
        }
        else
        {
            const QJsonValue oldVal = origSettings[key];
            const QJsonValueRef newVal = newSettings[key];

            if(oldVal.isObject() != newVal.isObject())
            {
                modifiedKeys.append(key);
                continue;
            }

            if(oldVal.isObject())
            {
                modifiedKeys.append(key);

                // Check sub object
                QJsonObject subObject = newVal.toObject();
                checkDifferences(oldVal.toObject(),
                                 subObject);

                // Overwrite with only changed keys
                newSettings[key] = subObject;
                continue;
            }

            if(oldVal.toVariant() != newVal.toVariant())
                modifiedKeys.append(key);
        }
    }

    return modifiedKeys;
}

void JSONDiff::applyDiffSub(QJsonObject &settings, const QJsonObject &modifiedSettings)
{
    for(const QString& key : modifiedSettings.keys())
    {
        const QJsonValue newVal = modifiedSettings[key];
        if(newVal.isObject())
        {
            QJsonObject resultObj = settings[key].toObject();
            applyDiffSub(resultObj, newVal.toObject());
            settings[key] = resultObj;
        }
        else
        {
            settings[key] = newVal;
        }
    }
}

void JSONDiff::applyDiff(QJsonObject &settings, const QJsonObject &modifiedSettings, const QStringList &modifiedKeys, const QString &namePrefix, const QString &nameSuffix)
{
    for(const QString& key : std::as_const(modifiedKeys))
    {
        // Set new value
        if(key == NameKey)
        {
            QString newName = namePrefix;
            newName.append(settings[NameKey].toString());
            newName.append(nameSuffix);
            settings[NameKey] = newName;
        }
        else
        {
            const QJsonValue newVal = modifiedSettings[key];
            if(newVal.isObject())
            {
                QJsonObject resultObj = settings[key].toObject();
                applyDiffSub(resultObj, newVal.toObject());
                settings[key] = resultObj;
            }
            else
            {
                settings[key] = newVal;
            }
        }
    }
}
