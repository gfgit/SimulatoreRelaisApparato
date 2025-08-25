#ifndef RIGHTCLICKEMULATORFILTER_H
#define RIGHTCLICKEMULATORFILTER_H

#include <QObject>

class RightClickEmulatorFilter : public QObject
{
    Q_OBJECT
public:
    explicit RightClickEmulatorFilter(QObject *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *e) override;
};

#endif // RIGHTCLICKEMULATORFILTER_H
