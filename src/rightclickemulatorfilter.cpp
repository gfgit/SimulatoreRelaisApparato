#include "rightclickemulatorfilter.h"

#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>


RightClickEmulatorFilter::RightClickEmulatorFilter(int &argc, char **argv)
    : QApplication(argc, argv)
{

}

bool RightClickEmulatorFilter::notify(QObject *watched, QEvent *e)
{
    if(e->spontaneous())
    {
        // Make Left click + Windows key -> Right click
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::MouseButtonDblClick:
        {
            QMouseEvent *ev = static_cast<QMouseEvent *>(e);
            if(!ev->modifiers().testFlag(Qt::AltModifier))
                break;

            // Tweak event if left button is pressed and right button is not
            bool needsTweak = (ev->button() == Qt::LeftButton || ev->buttons().testFlag(Qt::LeftButton))
                    && !ev->buttons().testFlag(Qt::RightButton);
            if(!needsTweak)
                break;

            // Fix mouse state
            Qt::MouseButton but = ev->button();
            if(but == Qt::LeftButton)
                but = Qt::RightButton;

            Qt::MouseButtons buts = ev->buttons();
            if(buts.testFlag(Qt::LeftButton))
            {
                buts.setFlag(Qt::RightButton, true);
                buts.setFlag(Qt::LeftButton, false);
            }

            QMouseEvent cloneEv(ev->type(), ev->position(), ev->scenePosition(), ev->globalPosition(),
                                but, buts, ev->modifiers(), ev->source(),
                                static_cast<const QPointingDevice *>(ev->device()));
            cloneEv.setTimestamp(ev->timestamp());
            cloneEv.setAccepted(ev->isAccepted());

            bool ret = QApplication::notify(watched, e);

            ev->setAccepted(cloneEv.isAccepted());

            // if(!QCoreApplication::instance()->notify(watched, &cloneEv))
            // {
            //     // Try again by sending directly
            //     watched->event(&cloneEv);
            // }

            // Eat original event
            return ret;
        }
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseRelease:
        case QEvent::GraphicsSceneMouseMove:
        case QEvent::GraphicsSceneMouseDoubleClick:
        {
            QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
            if(!ev->modifiers().testFlag(Qt::AltModifier))
                break;

            // Tweak event if left button is pressed and right button is not
            bool needsTweak = (ev->button() == Qt::LeftButton || ev->buttons().testFlag(Qt::LeftButton))
                    && !ev->buttons().testFlag(Qt::RightButton);
            if(!needsTweak)
                break;

            // Fix mouse state
            Qt::MouseButton but = ev->button();
            if(but == Qt::LeftButton)
                but = Qt::RightButton;

            Qt::MouseButtons buts = ev->buttons();
            if(buts.testFlag(Qt::LeftButton))
            {
                buts.setFlag(Qt::RightButton, true);
                buts.setFlag(Qt::LeftButton, false);
            }

            QGraphicsSceneMouseEvent cloneEv(ev->type());

            cloneEv.setTimestamp(ev->timestamp());
            cloneEv.setWidget(ev->widget());

            cloneEv.setPos(ev->pos());
            cloneEv.setScenePos(ev->scenePos());
            cloneEv.setScreenPos(ev->screenPos());

            cloneEv.setLastPos(ev->lastPos());
            cloneEv.setLastScenePos(ev->lastScenePos());
            cloneEv.setLastScreenPos(ev->lastScreenPos());

            cloneEv.setButtonDownPos(Qt::RightButton, ev->buttonDownPos(Qt::LeftButton));
            cloneEv.setButtonDownScenePos(Qt::RightButton, ev->buttonDownScenePos(Qt::LeftButton));
            cloneEv.setButtonDownScreenPos(Qt::RightButton, ev->buttonDownScreenPos(Qt::LeftButton));

            cloneEv.setButton(but);
            cloneEv.setButtons(buts);

            cloneEv.setModifiers(ev->modifiers());
            cloneEv.setSource(ev->source());
            cloneEv.setFlags(ev->flags());

            cloneEv.setAccepted(ev->isAccepted());

            bool ret = QApplication::notify(watched, e);

            ev->setAccepted(cloneEv.isAccepted());

            // if(!QCoreApplication::instance()->notify(watched, &cloneEv))
            // {
            //     // Try again by sending directly
            //     watched->event(&cloneEv);
            // }

            // Eat original event
            return ret;
        }
        default:
            break;
        }
    }

    return QApplication::notify(watched, e);
}
