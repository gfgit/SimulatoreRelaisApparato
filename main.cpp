#include "mainwindow.h"

#include <QApplication>

#include "nodes/onoffswitchnode.h"
#include "nodes/powersourcenode.h"
#include "nodes/simplecircuitnode.h"
#include "nodes/relaispowernode.h"
#include "nodes/relaiscontactnode.h"
#include "circuitcable.h"
#include "abstractrelais.h"

#include "graph/circuitscene.h"
#include "graph/cablegraphitem.h"
#include "graph/onoffgraphitem.h"
#include "graph/powersourcegraphitem.h"
#include "graph/relaispowergraphitem.h"
#include "graph/relaiscontactgraphitem.h"
#include "graph/simplenodegraphitem.h"

QPointF getConnectorPoint(AbstractNodeGraphItem *item, TileRotate r)
{
    const QPointF pos = item->pos();

    switch (toConnectorDirection(r))
    {
    case Connector::Direction::North:
        return {pos.x() + TileLocation::Size / 2.0, pos.y()};
    case Connector::Direction::South:
        return {pos.x() + TileLocation::Size / 2.0, pos.y() + TileLocation::Size};
    case Connector::Direction::East:
        return {pos.x() + TileLocation::Size, pos.y() + TileLocation::Size / 2.0};
    case Connector::Direction::West:
        return {pos.x(), pos.y() + TileLocation::Size / 2.0};
    default:
        break;
    }

    return QPointF();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* CIRCUIT SCHEME
     *
     * Source -|-> On/Off1 ------------|-> Relay
     *        (s1)                     |
     *         |-> On/Off2 -> On/Off3 -|
     *
     */

    PowerSourceNode powerSource;
    powerSource.setObjectName("PowerSource");

    SimpleCircuitNode s1;
    s1.setObjectName("s1");

    SimpleCircuitNode s2;
    s2.setObjectName("s2");
    s2.setDisabledContact(1);

    OnOffSwitchNode onOff1;
    onOff1.setObjectName("On1");

    OnOffSwitchNode onOff2;
    onOff2.setObjectName("On2");

    OnOffSwitchNode onOff3;
    onOff3.setObjectName("On3");

    RelaisPowerNode relPow1;
    relPow1.setObjectName("RelPow1");

    RelaisPowerNode relPow2;
    relPow2.setObjectName("RelPow2");

    RelaisContactNode relCont;
    relCont.setObjectName("RelCont1");

    AbstractRelais relay;
    relay.setName("C1");
    relay.addPowerNode(&relPow1);
    relay.addPowerNode(&relPow2);
    relay.addContactNode(&relCont);

    /*
    powerSource.setEnabled(true);
    onOff1.setOn(true);
    onOff1.setOn(false);

    powerSource.setEnabled(false);

    onOff2.setOn(true);
    powerSource.setEnabled(true);
    onOff3.setOn(true);
    onOff2.setOn(false);
    onOff1.setOn(true);
    */

    PowerSourceGraphItem *pwGraph = new PowerSourceGraphItem(&powerSource);
    pwGraph->setLocation(TileLocation{0, 1});

    SimpleNodeGraphItem *s1Graph = new SimpleNodeGraphItem(&s1);
    s1Graph->setLocation(TileLocation{0, 2});
    s1Graph->setRotate(TileRotate::Deg180);

    OnOffGraphItem *onOff1Graph = new OnOffGraphItem(&onOff1);
    onOff1Graph->setLocation(TileLocation{0, 3});
    onOff1Graph->setRotate(TileRotate::Deg180);

    OnOffGraphItem *onOff2Graph = new OnOffGraphItem(&onOff2);
    onOff2Graph->setLocation(TileLocation{2, 3});

    OnOffGraphItem *onOff3Graph = new OnOffGraphItem(&onOff3);
    onOff3Graph->setLocation(TileLocation{2, 4});
    onOff3Graph->setRotate(TileRotate::Deg180);

    SimpleNodeGraphItem *s2Graph = new SimpleNodeGraphItem(&s2);
    s2Graph->setLocation(TileLocation{0, 5});

    RelaisPowerGraphItem *relPowGraph1 = new RelaisPowerGraphItem(&relPow1);
    relPowGraph1->setLocation(TileLocation{0, 6});
    relPowGraph1->setRotate(TileRotate::Deg180);

    RelaisContactGraphItem *relContGraph1 = new RelaisContactGraphItem(&relCont);
    relContGraph1->setLocation(TileLocation{-2, 2});
    relContGraph1->setRotate(TileRotate::Deg270);

    RelaisPowerGraphItem *relPowGraph2 = new RelaisPowerGraphItem(&relPow2);
    relPowGraph2->setLocation(TileLocation{-2, 6});
    relPowGraph2->setRotate(TileRotate::Deg180);

    using Side = CircuitCable::Side;

    // c1 cable from power to s1
    CircuitCable c1;
    c1.setObjectName("c1");
    AbstractCircuitNode::CableItem conn;
    conn.cable = &c1;
    conn.cableSide = Side::A1;
    conn.nodeContact = 0;
    powerSource.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 1;
    powerSource.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    s1.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    s1.attachCable(conn);

    // c1 Graph
    CableGraphItem *c1Graph = new CableGraphItem(&c1);
    c1Graph->setToolTip("c1");
    c1Graph->setVisible(false);

    c1Graph->setPos(0, 0);
    QPainterPath pathC1;
    pathC1.moveTo(getConnectorPoint(pwGraph, pwGraph->rotate()));
    pathC1.lineTo(getConnectorPoint(s1Graph, s1Graph->rotate()));
    c1Graph->setPath(pathC1);

    // c2 cable from s1 to on/off1
    CircuitCable c2;
    c2.setObjectName("c2");
    conn.cable = &c2;
    conn.cableSide = Side::A1;
    conn.nodeContact = 4;
    s1.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 5;
    s1.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    onOff1.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    onOff1.attachCable(conn);

    // c2 Graph
    CableGraphItem *c2Graph = new CableGraphItem(&c2);
    c2Graph->setToolTip("c2");

    c2Graph->setPos(0, 0);
    QPainterPath pathC2;
    pathC2.moveTo(getConnectorPoint(s1Graph, TileRotate::Deg0));
    pathC2.lineTo(getConnectorPoint(onOff1Graph, TileRotate::Deg180));
    c2Graph->setPath(pathC2);
    c2Graph->setVisible(false);

    // c3 cable from on/off1 to s2
    CircuitCable c3;
    c3.setObjectName("c3");
    conn.cable = &c3;
    conn.cableSide = Side::A1;
    conn.nodeContact = 2;
    onOff1.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 3;
    onOff1.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 4;
    s2.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 5;
    s2.attachCable(conn);

    // c3 Graph
    CableGraphItem *c3Graph = new CableGraphItem(&c3);
    c3Graph->setToolTip("c3");

    c3Graph->setPos(0, 0);
    QPainterPath pathC3;
    pathC3.moveTo(getConnectorPoint(onOff1Graph, TileRotate::Deg0));
    pathC3.lineTo(getConnectorPoint(s2Graph, TileRotate::Deg180));
    c3Graph->setPath(pathC3);

    // c4 cable from s1 to on/off2
    CircuitCable c4;
    c4.setObjectName("c4");
    conn.cable = &c4;
    conn.cableSide = Side::A1;
    conn.nodeContact = 2;
    s1.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 3;
    s1.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    onOff2.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    onOff2.attachCable(conn);

    // c4 Graph
    CableGraphItem *c4Graph = new CableGraphItem(&c4);
    c4Graph->setToolTip("c4");

    c4Graph->setPos(0, 0);
    QPainterPath pathC4;
    auto startC4 = getConnectorPoint(s1Graph, TileRotate::Deg270);
    auto endC4 = getConnectorPoint(onOff2Graph, TileRotate::Deg180);
    pathC4.moveTo(startC4);
    pathC4.lineTo(endC4.x(), startC4.y());
    pathC4.lineTo(endC4);
    c4Graph->setPath(pathC4);

    // c5 cable from on/off2 to on/off3
    CircuitCable c5;
    c5.setObjectName("c5");
    conn.cable = &c5;
    conn.cableSide = Side::A1;
    conn.nodeContact = 2;
    onOff2.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 3;
    onOff2.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    onOff3.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    onOff3.attachCable(conn);

    // c5 Graph
    CableGraphItem *c5Graph = new CableGraphItem(&c5);
    c5Graph->setToolTip("c5");
    c5Graph->setVisible(false); // TODO

    // c6 cable from on/off3 to s2
    CircuitCable c6;
    c6.setObjectName("c6");
    conn.cable = &c6;
    conn.cableSide = Side::A1;
    conn.nodeContact = 2;
    onOff3.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 3;
    onOff3.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 6;
    s2.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 7;
    s2.attachCable(conn);

    // c6 Graph
    CableGraphItem *c6Graph = new CableGraphItem(&c6);
    c6Graph->setToolTip("c6");

    c6Graph->setPos(0, 0);
    QPainterPath pathC6;
    auto startC6 = getConnectorPoint(s2Graph, TileRotate::Deg270);
    auto endC6 = getConnectorPoint(onOff3Graph, TileRotate::Deg0);
    pathC6.moveTo(startC6);
    pathC6.lineTo(endC6.x(), startC6.y());
    pathC6.lineTo(endC6);
    c6Graph->setPath(pathC6);

    // c7 cable from s1 to relay contact common
    CircuitCable c7;
    c7.setObjectName("c7");
    conn.cable = &c7;
    conn.cableSide = Side::A1;
    conn.nodeContact = 6;
    s1.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 7;
    s1.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    relCont.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    relCont.attachCable(conn);

    // c7 Graph
    CableGraphItem *c7Graph = new CableGraphItem(&c7);
    c7Graph->setToolTip("c7");

    c7Graph->setPos(0, 0);
    QPainterPath pathC7;
    auto startC7 = getConnectorPoint(relContGraph1, TileRotate::Deg270);
    auto endC7 = getConnectorPoint(s1Graph, TileRotate::Deg90);
    pathC7.moveTo(startC7);
    pathC7.lineTo(endC7);
    c7Graph->setPath(pathC7);

    // c8 cable from relay contact up to relay power 2
    CircuitCable c8;
    c8.setObjectName("c8");
    conn.cable = &c8;
    conn.cableSide = Side::A1;
    conn.nodeContact = 4;
    relCont.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 5;
    relCont.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    relPow2.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    relPow2.attachCable(conn);

    // c8 Graph
    CableGraphItem *c8Graph = new CableGraphItem(&c8);
    c8Graph->setToolTip("c8");

    c8Graph->setPos(0, 0);
    QPainterPath pathC8;
    auto startC8 = getConnectorPoint(relContGraph1, TileRotate::Deg0);
    auto endC8 = getConnectorPoint(relPowGraph2, TileRotate::Deg180);
    pathC8.moveTo(startC8);
    pathC8.lineTo(endC8);
    c8Graph->setPath(pathC8);

    // c9 cable from s2 to relay power 1
    CircuitCable c9;
    c9.setObjectName("c9");
    conn.cable = &c9;
    conn.cableSide = Side::A1;
    conn.nodeContact = 0;
    s2.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 1;
    s2.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    relPow1.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    relPow1.attachCable(conn);

    // c8 Graph
    CableGraphItem *c9Graph = new CableGraphItem(&c9);
    c9Graph->setToolTip("c9");
    c9Graph->setVisible(false);

    auto guard = qScopeGuard([&powerSource](){powerSource.setEnabled(false);});

    MainWindow w;

    CircuitScene& scene = *w.scene();
    scene.addNode(pwGraph);
    scene.addNode(onOff1Graph);
    scene.addNode(onOff2Graph);
    scene.addNode(onOff3Graph);
    scene.addNode(relPowGraph1);
    scene.addNode(s1Graph);
    scene.addNode(s2Graph);

    scene.addNode(relContGraph1);
    scene.addNode(relPowGraph2);

    scene.addItem(c1Graph);
    scene.addItem(c2Graph);
    scene.addItem(c3Graph);
    scene.addItem(c4Graph);
    scene.addItem(c5Graph);
    scene.addItem(c6Graph);
    scene.addItem(c7Graph);
    scene.addItem(c8Graph);
    scene.addItem(c9Graph);

    w.show();
    return a.exec();
}
