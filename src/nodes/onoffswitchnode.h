/**
 * src/nodes/onoffswitchnode.h
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

#ifndef ONOFFSWITCHNODE_H
#define ONOFFSWITCHNODE_H

#include "abstractcircuitnode.h"

class OnOffSwitchNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit OnOffSwitchNode(QObject *parent = nullptr);

    virtual QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    static constexpr QLatin1String NodeType = QLatin1String("on_off_switch");
    QString nodeType() const override;

    bool isOn() const;
    void setOn(bool newOn);

signals:
    void isOnChanged(bool on);

private:
    bool m_isOn = false;
};

#endif // ONOFFSWITCHNODE_H
