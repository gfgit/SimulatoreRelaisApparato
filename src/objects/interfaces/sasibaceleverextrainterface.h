/**
 * src/objects/interfaces/sasibaceleverextrainterface.h
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

#ifndef SASIBACELEVEREXTRAINTERFACE_H
#define SASIBACELEVEREXTRAINTERFACE_H

#include "abstractobjectinterface.h"

class SasibACELeverExtraInterface : public AbstractObjectInterface
{
public:
    enum class Button
    {
        Left = 0,
        Right,
        NButtons
    };

    // Property names
    static constexpr QLatin1String LeftButPropName = QLatin1String("left_but");
    static constexpr QLatin1String RightButPropName = QLatin1String("right_but");

    SasibACELeverExtraInterface(AbstractSimulationObject *obj);
    ~SasibACELeverExtraInterface();

    static constexpr QLatin1String IfaceType = QLatin1String("sasib_lever_extra");
    QString ifaceType() override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    AbstractSimulationObject *getButton(Button whichBut) const;
    void setButton(AbstractSimulationObject *newLeftButton, Button whichBut);

    void setButtonLocked(bool lock, Button whichBut);

    bool rightButtonSwitchElectroMagnet() const;
    void setRightButtonSwitchElectroMagnet(bool val);

    void updateMagnetState();

protected:
    void onTrackedObjectDestroyed(AbstractSimulationObject *obj) override;

private:
    AbstractSimulationObject *mLeftButton = nullptr;
    AbstractSimulationObject *mRightButton = nullptr;

    bool mRightButtonSwitchElectroMagnet = false;
};

#endif // SASIBACELEVEREXTRAINTERFACE_H
