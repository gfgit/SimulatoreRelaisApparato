/**
 * src/circuits/graphs/relaispowergraphitem.h
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

#ifndef RELAISPOWERGRAPHITEM_H
#define RELAISPOWERGRAPHITEM_H


#include "abstractnodegraphitem.h"

class RelaisPowerNode;
class AbstractRelais;

class RelaisPowerGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef RelaisPowerNode Node;

    RelaisPowerGraphItem(RelaisPowerNode *node_);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    void getConnectors(std::vector<Connector>& connectors) const final;

    QString displayString() const override;

    QString tooltipString() const override;

    QRectF textDisplayRect() const override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    RelaisPowerNode *node() const;

    inline TileRotate twoConnectorsRotate() const
    {
        // Always put connectors horizontal
        // We ignore other rotations otherwise we cannot draw name :(
        TileRotate r = rotate();
        if(r == TileRotate::Deg90)
            r = TileRotate::Deg0;
        else if(r == TileRotate::Deg270)
            r = TileRotate::Deg180;
        return r;
    }

    void setArrowDirection(Connector::Direction newArrowDirection);

    bool preferEastArrow() const;
    void setPreferEastArrow(bool newPreferEastArrow);

protected:
    void recalculateTextPosition() override;

private slots:
    void updateRelay();
    void onRelayTypeChanged();

protected slots:
    void updateName() override;

private:
    QRectF arrowDisplayRect() const;

    void drawRelayArrow(QPainter *painter);

private:
    static constexpr double relayRadius = 50 - 10/2; // Half rect - half pen width

    AbstractRelais *mRelay = nullptr;
    Connector::Direction mArrowDirection = Connector::Direction::West;
    bool mPreferEastArrow = false;
};

#endif // RELAISPOWERGRAPHITEM_H
