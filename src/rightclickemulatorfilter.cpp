#include "rightclickemulatorfilter.h"

#include <QMouseEvent>


RightClickEmulatorFilter::RightClickEmulatorFilter(QObject *parent)
    : QObject{parent}
{

}

bool RightClickEmulatorFilter::eventFilter(QObject *watched, QEvent *e)
{
    if(watched->isWidgetType() && e->spontaneous())
    {
        // Make Left click + Windows key -> Right click
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
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

            if(!QCoreApplication::instance()->notify(watched, &cloneEv))
            {
                // Try again by sending directly
                watched->event(&cloneEv);
            }

            // Eat original event
            return true;
        }
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, e);
}
