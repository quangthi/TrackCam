#include "qcustomcheckbox.h"

QCustomCheckBox::QCustomCheckBox(QWidget *parent) : QCheckBox(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomCheckBox::resetView()
{
    this->setStyleSheet("color:rgb(255, 255, 255);"
                        "border:none;"
                        "border-radius: 5px;"
                        "font:  11pt \"MS Shell Dlg 2\";"
                        );

    repaint();

}

void QCustomCheckBox::highLight()
{
    this->setStyleSheet("color:rgb(255, 255, 255);"
                        "border : none;"
                        "border-radius: 5px;"
                        "font: bold 11pt \"MS Shell Dlg 2\";");

    repaint();
}

void QCustomCheckBox::hoverEnter(QHoverEvent *)
{

    highLight();
}

void QCustomCheckBox::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QCustomCheckBox::hoverMove(QHoverEvent *)
{
    highLight();

}

bool QCustomCheckBox::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(event));
        return true;
        break;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(event));
        return true;
        break;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(event));
        return true;
        break;
    default:
        break;
    }
    return QWidget::event(event);
}
