/**
 * src/circuits/nodes/electromagnetcontactnode.h
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

#ifndef ELECTROMAGNET_CONTACT_NODE_H
#define ELECTROMAGNET_CONTACT_NODE_H

#include "abstractdeviatornode.h"

class ElectroMagnetObject;

class ElectromagnetContactNode : public AbstractDeviatorNode
{
    Q_OBJECT
public:
    explicit ElectromagnetContactNode(ModeManager *mgr, QObject *parent = nullptr);
    ~ElectromagnetContactNode();

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    static constexpr QLatin1String NodeType = QLatin1String("magnet_contact");
    QString nodeType() const override;

    ElectroMagnetObject *magnet() const;
    void setMagnet(ElectroMagnetObject *newMagnet);

signals:
    void magnetChanged(ElectroMagnetObject *obj);

private:
    friend class ElectroMagnetObject;
    void refreshContactState();

private:
    ElectroMagnetObject *mMagnet = nullptr;
};

#endif // ELECTROMAGNET_CONTACT_NODE_H
