#ifndef QCUSTOMEDIT_H
#define QCUSTOMEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <qevent.h>

class QCustomEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit QCustomEdit(QWidget *parent = 0);

protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);
};

#endif // QCUSTOMEDIT_H
