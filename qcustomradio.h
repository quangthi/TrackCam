#ifndef QCUSTOMRADIO_H
#define QCUSTOMRADIO_H

#include <QWidget>
#include <QRadioButton>
#include <qevent.h>

class QCustomRadio : public QRadioButton
{
    Q_OBJECT
public:
    explicit QCustomRadio(QWidget *parent = 0);

protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);
};

#endif // QCUSTOMRADIO_H
