#ifndef QCUSTOMCHECKBOX_H
#define QCUSTOMCHECKBOX_H

#include <QWidget>
#include <QCheckBox>
#include <qevent.h>


class QCustomCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit QCustomCheckBox(QWidget *parent = 0);

protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);
};

#endif // QCUSTOMCHECKBOX_H
