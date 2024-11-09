/**
 * src/circuits/nodes/abstractdeviatornode.h
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

#ifndef ABSTRACTDEVIATORNODE_H
#define ABSTRACTDEVIATORNODE_H

#include "abstractcircuitnode.h"

class AbstractDeviatorNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    enum ContactIdx
    {
        CommonIdx = 0,
        UpIdx = 1,
        DownIdx = 2,
        NContacts
    };

    AbstractDeviatorNode(ModeManager *mgr, QObject *parent = nullptr);

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    // Settings
    bool swapContactState() const;
    void setSwapContactState(bool newSwapContactState);

    bool flipContact() const;
    void setFlipContact(bool newFlipContact);

    bool hasCentralConnector() const;
    void setHasCentralConnector(bool newHasCentralConnector);

    bool isContactOn(int contact) const;

    bool allowSwap() const;

    bool canChangeCentralConnector() const;
    void setCanChangeCentralConnector(bool newCanChangeCentralConnector);

signals:
    void deviatorStateChanged();

protected:
    void setContactState(bool valUp, bool valDown);
    void setAllowSwap(bool newAllowSwap);

private:
    bool mFlipContact = false;
    bool mSwapContactState = false;
    bool mHasCentralConnector = true; // Up connector
    bool mAllowSwap = true;
    bool mCanChangeCentralConnector = true;

    // UpIdx, DownIdx reflecting actual state
    bool mContactOnArr[2] = {false, false};
};

#endif // ABSTRACTDEVIATORNODE_H
