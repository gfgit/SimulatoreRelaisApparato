/**
 * src/circuits/nodes/buttoncontactnode.h
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

#ifndef BUTTON_CONTACT_NODE_H
#define BUTTON_CONTACT_NODE_H

#include "abstractdeviatornode.h"

class AbstractSimulationObject;
class ButtonInterface;

class ButtonContactNode : public AbstractDeviatorNode
{
    Q_OBJECT
public:
    explicit ButtonContactNode(ModeManager *mgr, QObject *parent = nullptr);
    ~ButtonContactNode();

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    static constexpr QLatin1String NodeType = QLatin1String("button_contact");
    QString nodeType() const override;

    AbstractSimulationObject *button() const;
    void setButton(AbstractSimulationObject *newButton);

    ButtonInterface *buttonIface() const;

    inline bool getContactStateFor(int buttonState, int contact) const
    {
        Q_ASSERT(buttonState >= 0 && buttonState < 3);
        Q_ASSERT(contact == 0 || contact == 1);

        if(contact == 0)
            return mFirstContactState[buttonState];
        return mSecondContactState[buttonState];
    }

    void setContactStateFor(int buttonState, int contact, bool val);

signals:
    void buttonChanged(AbstractSimulationObject *obj);
    void contactStateSettingsChanged();

private slots:
    void onInterfacePropertyChanged(const QString &ifaceName,
                                    const QString &propName);

private:
    void refreshContactState();

private:
    AbstractSimulationObject *mButton = nullptr;
    ButtonInterface *mButtonIface = nullptr;

    bool mFirstContactState[3] = {false, false, false};
    bool mSecondContactState[3] = {false, false, false};
};

#endif // BUTTON_CONTACT_NODE_H
