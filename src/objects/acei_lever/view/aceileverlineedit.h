/**
 * src/objects/acei_lever/view/aceileverlineedit.h
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

#ifndef ACEI_LEVER_LINEEDIT_H
#define ACEI_LEVER_LINEEDIT_H

#include <QLineEdit>

class ACEILeverModel;
class ACEILeverObject;

class ACEILeverLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    ACEILeverLineEdit(ACEILeverModel *m, QWidget *parent = nullptr);

    ACEILeverObject *relais() const
    {
        return mLever;
    }

public slots:
    void setLever(ACEILeverObject *newLever);

signals:
    void leverChanged(ACEILeverObject *r);

private:
    ACEILeverModel *mACEILeverModel = nullptr;
    ACEILeverObject *mLever = nullptr;
};


#endif // ACEI_LEVER_LINEEDIT_H
