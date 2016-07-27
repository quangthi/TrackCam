#include "qcustomradio.h"

QCustomRadio::QCustomRadio(QWidget *parent) : QRadioButton(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomRadio::resetView()
{
    this->setStyleSheet("color:rgb(255, 255, 255);"
                        "border:none;"
                        "border-radius: 5px;"
                        "font:  11pt \"MS Shell Dlg 2\";"
                        );

    repaint();

}

void QCustomRadio::highLight()
{
    this->setStyleSheet("color:rgb(255, 255, 255);"
                        "border : none;"
                        "border-radius: 5px;"
                        "font: bold 11pt \"MS Shell Dlg 2\";");

    repaint();
}

void QCustomRadio::hoverEnter(QHoverEvent *)
{

    highLight();
}

void QCustomRadio::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QCustomRadio::hoverMove(QHoverEvent *)
{
    highLight();

}

bool QCustomRadio::event(QEvent *event)
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
