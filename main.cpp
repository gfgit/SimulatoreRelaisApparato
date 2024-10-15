#include "mainwindow.h"

#include <QApplication>

#include "nodes/onoffswitchnode.h"
#include "nodes/powersourcenode.h"
#include "nodes/simplecircuitnode.h"
#include "nodes/relaispowernode.h"
#include "nodes/relaiscontactnode.h"
#include "circuitcable.h"
#include "abstractrelais.h"

#include <QGraphicsScene>
#include "graph/cablegraphitem.h"
#include "graph/onoffgraphitem.h"
#include "graph/powersourcegraphitem.h"
#include "graph/relaispowergraphitem.h"
#include "graph/relaiscontactgraphitem.h"

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

    OnOffSwitchNode onOff1;
    onOff1.setObjectName("OnOff 1");

    OnOffSwitchNode onOff2;
    onOff2.setObjectName("OnOff 2");

    OnOffSwitchNode onOff3;
    onOff3.setObjectName("OnOff 3");

    RelaisPowerNode relPow1;
    relPow1.setObjectName("RelPow1");

    RelaisPowerNode relPow2;
    relPow2.setObjectName("RelPow2");

    RelaisContactNode relCont;
    relCont.setObjectName("RelCont1");

    AbstractRelais relay;
    relay.addPowerNode(&relPow1);
    relay.addPowerNode(&relPow2);
    relay.addContactNode(&relCont);

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

    // c2 cable from s1 to on/off1
    CircuitCable c2;
    c2.setObjectName("c2");
    conn.cable = &c2;
    conn.cableSide = Side::A1;
    conn.nodeContact = 0;
    s1.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 1;
    s1.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    onOff1.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    onOff1.attachCable(conn);

    // c3 cable from on/off1 to relay power
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
    conn.nodeContact = 0;
    relPow1.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    relPow1.attachCable(conn);

    // c4 cable from s1 to on/off2
    CircuitCable c4;
    c4.setObjectName("c4");
    conn.cable = &c4;
    conn.cableSide = Side::A1;
    conn.nodeContact = 0;
    s1.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 1;
    s1.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    onOff2.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    onOff2.attachCable(conn);

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

    // c6 cable from on/off3 to relay power
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
    conn.nodeContact = 0;
    relPow1.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    relPow1.attachCable(conn);

    // c7 cable from s1 to relay contact common
    CircuitCable c7;
    c7.setObjectName("c7");
    conn.cable = &c7;
    conn.cableSide = Side::A1;
    conn.nodeContact = 0;
    s1.attachCable(conn);

    conn.cableSide = Side::A2;
    conn.nodeContact = 1;
    s1.attachCable(conn);

    conn.cableSide = Side::B1;
    conn.nodeContact = 0;
    relCont.attachCable(conn);

    conn.cableSide = Side::B2;
    conn.nodeContact = 1;
    relCont.attachCable(conn);

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

    QGraphicsScene scene;

    PowerSourceGraphItem *pwGraph = new PowerSourceGraphItem(&powerSource);
    pwGraph->setPos(0, 0);

    CableGraphItem *c1Graph = new CableGraphItem(&c1);
    c1Graph->setPos(25, 50);
    c1Graph->setToolTip("c1");

    CableGraphItem *c2Graph = new CableGraphItem(&c2);
    c2Graph->setPos(25, 100);
    c2Graph->setToolTip("c2");

    CableGraphItem *c3Graph = new CableGraphItem(&c3);
    c3Graph->setPos(25, 200);
    c3Graph->setToolTip("c3");

    QPainterPath pathC3;
    pathC3.lineTo(0, 150);
    c3Graph->setPath(pathC3);

    CableGraphItem *c4Graph = new CableGraphItem(&c4);
    c4Graph->setPos(25, 100);
    c4Graph->setToolTip("c4");

    QPainterPath pathC4;
    pathC4.lineTo(100, 0);
    pathC4.lineTo(100, 50);
    c4Graph->setPath(pathC4);

    CableGraphItem *c5Graph = new CableGraphItem(&c5);
    c5Graph->setPos(125, 200);
    c5Graph->setToolTip("c5");

    CableGraphItem *c6Graph = new CableGraphItem(&c6);
    c6Graph->setPos(25, 300);
    c6Graph->setToolTip("c6");

    QPainterPath pathC6;
    pathC6.moveTo(100, 0);
    pathC6.lineTo(100, 25);
    pathC6.lineTo(25, 25);
    pathC6.lineTo(0, 50);
    c6Graph->setPath(pathC6);

    OnOffGraphItem *onOff1Graph = new OnOffGraphItem(&onOff1);
    onOff1Graph->setPos(0, 150);

    OnOffGraphItem *onOff2Graph = new OnOffGraphItem(&onOff2);
    onOff2Graph->setPos(100, 150);

    OnOffGraphItem *onOff3Graph = new OnOffGraphItem(&onOff3);
    onOff3Graph->setPos(100, 250);

    RelaisPowerGraphItem *relPowGraph1 = new RelaisPowerGraphItem(&relPow1);
    relPowGraph1->setPos(0, 350);

    RelaisContactGraphItem *relContGraph1 = new RelaisContactGraphItem(&relCont);
    relContGraph1->setPos(-100, 250);

    RelaisPowerGraphItem *relPowGraph2 = new RelaisPowerGraphItem(&relPow2);
    relPowGraph2->setPos(-100, 350);

    CableGraphItem *c7Graph = new CableGraphItem(&c7);
    c7Graph->setPos(25, 100);
    c7Graph->setToolTip("c7");

    QPainterPath pathC7;
    pathC7.lineTo(-100, 0);
    pathC7.lineTo(-100, 150);
    c7Graph->setPath(pathC7);

    CableGraphItem *c8Graph = new CableGraphItem(&c8);
    c8Graph->setPos(-75, 300);
    c8Graph->setToolTip("c8");

    scene.addItem(pwGraph);
    scene.addItem(c1Graph);
    scene.addItem(c2Graph);
    scene.addItem(c3Graph);
    scene.addItem(c4Graph);
    scene.addItem(c5Graph);
    scene.addItem(c6Graph);
    scene.addItem(onOff1Graph);
    scene.addItem(onOff2Graph);
    scene.addItem(onOff3Graph);
    scene.addItem(relPowGraph1);

    scene.addItem(c7Graph);
    scene.addItem(c8Graph);
    scene.addItem(relContGraph1);
    scene.addItem(relPowGraph2);


    MainWindow w;
    w.setScene(&scene);
    w.show();
    return a.exec();
}
