/**
 * src/circuits/nodes/screenrelaiscontactnode.h
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

#ifndef SCREEN_RELAIS_CONTACT_NODE_H
#define SCREEN_RELAIS_CONTACT_NODE_H

#include "abstractdeviatornode.h"

class ScreenRelais;

class ScreenRelaisContactNode : public AbstractDeviatorNode
{
    Q_OBJECT
public:
    enum class ContactState
    {
        Straight = 0,
        Middle = 1,
        Reversed = 2
    };

    explicit ScreenRelaisContactNode(ModeManager *mgr, QObject *parent = nullptr);
    ~ScreenRelaisContactNode();

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("screen_relais_contact");
    QString nodeType() const override;

    ScreenRelais *screenRelais() const;
    void setScreenRelais(ScreenRelais *newRelais);

    ContactState state() const;
    void setState(ContactState newState);

    bool isContactA() const;
    void setIsContactA(bool newIsContactA);

signals:
    void relayChanged(ScreenRelais *r);

private:
    ScreenRelais *mScreenRelais = nullptr;
    ContactState mState = ContactState::Middle;

    bool mIsContactA = true;
};

#endif // SCREEN_RELAIS_CONTACT_NODE_H
