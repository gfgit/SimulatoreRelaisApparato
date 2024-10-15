#include "abstractnodegraphitem.h"

#include "../abstractcircuitnode.h"

#include <QPainter>
#include <QFont>

AbstractNodeGraphItem::AbstractNodeGraphItem(AbstractCircuitNode *node_)
    : QGraphicsObject()
    , mAbstractNode(node_)
{
    setParent(mAbstractNode);

    connect(mAbstractNode, &QObject::objectNameChanged,
            this, &AbstractNodeGraphItem::updateName);

    updateName();
}

QRectF AbstractNodeGraphItem::boundingRect() const
{
    return QRectF(0, 0, TileLocation::Size, TileLocation::Size);
}

void AbstractNodeGraphItem::triggerUpdate()
{
    update();
}

void AbstractNodeGraphItem::updateName()
{
    setToolTip(mAbstractNode->objectName());
}

void AbstractNodeGraphItem::drawMorsetti(QPainter *painter, bool on, const QString& name1, const QString& name2, TileRotate r)
{
    QLineF morsettoLine;

    QRectF morsettoEllipse;
    morsettoEllipse.setSize(QSizeF(13.0, 13.0));

    QRectF textRect1;
    textRect1.setSize(QSizeF(40.0, 22.0));

    QRectF textRect2 = textRect1;

    Qt::Alignment text1Align = Qt::AlignVCenter;
    Qt::Alignment text2Align = Qt::AlignVCenter;

    switch (toConnectorDirection(r))
    {
    case Connector::Direction::North:
        morsettoLine.setP1(QPointF(TileLocation::Size / 2.0,  0.0));
        morsettoLine.setP2(QPointF(TileLocation::Size / 2.0, 22.0));
        morsettoEllipse.moveCenter(QPointF(TileLocation::Size / 2.0, 11.0));

        textRect1.moveTopLeft(QPointF(1.0, 0.0));
        textRect2.moveTopRight(QPointF(TileLocation::Size - 1.0, 0.0));

        text1Align |= Qt::AlignRight;
        text2Align |= Qt::AlignLeft;
        break;

    case Connector::Direction::South:
        morsettoLine.setP1(QPointF(TileLocation::Size / 2.0,
                                   TileLocation::Size));
        morsettoLine.setP2(QPointF(TileLocation::Size / 2.0,
                                   TileLocation::Size - 22.0));
        morsettoEllipse.moveCenter(QPointF(TileLocation::Size / 2.0,
                                           TileLocation::Size - 11.0));

        // Text 1 is on the right
        textRect2.moveBottomLeft(QPointF(1.0,
                                         TileLocation::Size));
        textRect1.moveBottomRight(QPointF(TileLocation::Size - 1.0,
                                          TileLocation::Size));

        text1Align |= Qt::AlignLeft;
        text2Align |= Qt::AlignRight;
        break;

    case Connector::Direction::East:
        morsettoLine.setP1(QPointF(TileLocation::Size - 22.0,
                                   TileLocation::Size / 2.0));
        morsettoLine.setP2(QPointF(TileLocation::Size,
                                   TileLocation::Size / 2.0));
        morsettoEllipse.moveCenter(QPointF(TileLocation::Size - 11.0,
                                           TileLocation::Size / 2.0));

        // Text 1 goes above Text 2
        textRect1.moveBottomRight(QPointF(TileLocation::Size - 1.0,
                                          TileLocation::Size / 2.0 - 6.0));
        textRect2.moveTopRight(QPointF(TileLocation::Size - 1.0,
                                       TileLocation::Size / 2.0 + 6.0));

        text1Align |= Qt::AlignRight;
        text2Align |= Qt::AlignRight;
        break;

    case Connector::Direction::West:
        morsettoLine.setP1(QPointF(0.0,
                                   TileLocation::Size / 2.0));
        morsettoLine.setP2(QPointF(22.0,
                                   TileLocation::Size / 2.0));
        morsettoEllipse.moveCenter(QPointF(11.0,
                                           TileLocation::Size / 2.0));

        // Text 1 goes below Text 2
        textRect2.moveBottomLeft(QPointF(1.0,
                                         TileLocation::Size / 2.0 - 6.0));
        textRect1.moveTopLeft(QPointF(1.0,
                                      TileLocation::Size / 2.0 + 6.0));

        text1Align |= Qt::AlignLeft;
        text2Align |= Qt::AlignLeft;
        break;
    default:
        break;
    }

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(5.0);
    pen.setColor(on ? Qt::red : Qt::black);

    painter->setPen(pen);
    painter->drawLine(morsettoLine);

    painter->setPen(Qt::NoPen);
    painter->setBrush(on ? Qt::red : Qt::black);
    painter->drawEllipse(morsettoEllipse);

    QFont f;
    f.setPointSizeF(12.0);
    f.setItalic(true);
    painter->setFont(f);

    painter->setPen(Qt::black);
    painter->drawText(textRect1, name1, text1Align);

    painter->drawText(textRect2, name2, text2Align);
}

void AbstractNodeGraphItem::drawName(QPainter *painter, const QString& name, TileRotate r)
{
    QRectF textRect;

    Qt::Alignment textAlign = Qt::AlignVCenter;

    switch (toConnectorDirection(r - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        textRect.setLeft(26.0);
        textRect.setWidth(50.0);
        textRect.setBottom(TileLocation::Size / 2.0 - 12.0);
        textRect.setTop(5.0);

        textAlign |= Qt::AlignLeft;
        break;

    case Connector::Direction::South:
        textRect.setLeft(26.0);
        textRect.setWidth(50.0);
        textRect.setTop(TileLocation::Size / 2.0 + 12.0);
        textRect.setBottom(TileLocation::Size - 5.0);

        textAlign |= Qt::AlignLeft;
        break;

    case Connector::Direction::East:
        textRect.setLeft(TileLocation::Size / 2.0 + 3.0);
        textRect.setRight(TileLocation::Size - 5.0);
        textRect.setTop(23.0);
        textRect.setBottom(TileLocation::Size / 2.0);

        textAlign |= Qt::AlignRight;
        break;

    case Connector::Direction::West:
        textRect.setLeft(3.0);
        textRect.setRight(TileLocation::Size / 2.0 - 5.0);
        textRect.setTop(23.0);
        textRect.setBottom(TileLocation::Size / 2.0);

        textAlign |= Qt::AlignLeft;
        break;
    default:
        break;
    }

    QFont f;
    f.setPointSizeF(18.0);
    f.setBold(true);

    QFontMetrics metrics(f, painter->device());
    double width = metrics.horizontalAdvance(name, QTextOption(textAlign));
    if(width > textRect.width())
    {
        f.setPointSizeF(f.pointSizeF() * textRect.width() / width);
    }

    painter->setFont(f);
    painter->drawText(textRect, name, textAlign);
}

TileRotate AbstractNodeGraphItem::rotate() const
{
    return mRotate;
}

void AbstractNodeGraphItem::setRotate(TileRotate newRotate)
{
    if(mRotate == newRotate)
        return;
    mRotate = newRotate;
    update();
}
