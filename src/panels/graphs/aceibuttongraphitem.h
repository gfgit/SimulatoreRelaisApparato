/**
 * src/panels/graphs/aceibuttongraphitem.h
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

#ifndef ACEIBUTTONGRAPHITEM_H
#define ACEIBUTTONGRAPHITEM_H

#include "../abstractnodegraphitem.h"

#include "../../nodes/onoffswitchnode.h"

class AbstractSimulationObject;
class ButtonInterface;

class LightBulbObject;

// TODO: this is a fake node
class FakeACEIButtonNode : public OnOffSwitchNode
{
public:
    explicit FakeACEIButtonNode(ModeManager *mgr, QObject *parent = nullptr)
        : OnOffSwitchNode(mgr, parent)
    {

    }

    static constexpr QLatin1String NodeType = QLatin1String("acei_button");
    QString nodeType() const override;
};

class ACEIButtonGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef FakeACEIButtonNode Node;
    static constexpr QLatin1String CustomNodeType = QLatin1String("acei_button");

    explicit ACEIButtonGraphItem(OnOffSwitchNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    AbstractSimulationObject *button() const;
    void setButton(AbstractSimulationObject *newButton);

    LightBulbObject *centralLight() const;
    void setCentralLight(LightBulbObject *newCentralLight);

signals:
    void buttonChanged(AbstractSimulationObject *newButton);
    void lightsChanged();

private slots:
    void onButtonDestroyed();
    void onLightDestroyed();

    void onInterfacePropertyChanged(const QString &ifaceName,
                                    const QString &propName,
                                    const QVariant &value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

private:
    AbstractSimulationObject *mButton = nullptr;
    ButtonInterface *mButtonIface = nullptr;
    LightBulbObject *mCentralLight = nullptr;
};

#endif // ACEIBUTTONGRAPHITEM_H
