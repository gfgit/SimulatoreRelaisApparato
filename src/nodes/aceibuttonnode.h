/**
 * src/nodes/aceibuttonnode.h
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

#ifndef ACEIBUTTONNODE_H
#define ACEIBUTTONNODE_H

#include "abstractcircuitnode.h"

class ACEIButtonNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    enum State
    {
        Normal = 0,
        Pressed,
        Extracted
    };

    explicit ACEIButtonNode(QObject *parent = nullptr);
    ~ACEIButtonNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("acei_button");
    QString nodeType() const override;

    State state() const;
    void setState(State newState);

    bool flipContact() const;
    void setFlipContact(bool newFlipContact);

signals:
    void stateChanged();

private:
    State mState = State::Normal;
    bool mFlipContact = false;
};

#endif // ACEIBUTTONNODE_H
