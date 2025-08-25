#ifndef RIGHTCLICKEMULATORFILTER_H
#define RIGHTCLICKEMULATORFILTER_H

#include <QApplication>

class RightClickEmulatorFilter : public QApplication
{
    Q_OBJECT
public:
    explicit RightClickEmulatorFilter(int &argc, char **argv);

    bool notify(QObject *watched, QEvent *e) override;
};

#endif // RIGHTCLICKEMULATORFILTER_H
