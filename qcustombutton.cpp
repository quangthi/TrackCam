#include "qcustombutton.h"

QCustomButton::QCustomButton(QWidget *parent) : QToolButton(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomButton::hoverEnter(QHoverEvent *)
{

    highLight();
}

void QCustomButton::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QCustomButton::hoverMove(QHoverEvent *)
{
    highLight();

}
void QCustomButton::highLight()
{
    //this->setStyleSheet("background-color: rgb(41, 84, 118);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";border: none;border-radius: 5px;padding: 7px; ");
    this->setStyleSheet("background-color: rgb(41, 84, 118);"
                        "color:rgb(255, 255, 255);"
                        "font: bold 12pt \"MS Shell Dlg 2\";"
                        "border-radius: 5px;"
                        "border-color: beige;"
                        "border-style: outset;"
                         "border-width: 1px;");


    //if(QToolButton::isChecked())this->setStyleSheet("color: #fff;border: none;border-radius: 5px;padding: 10px;background: qradialgradient(cx: 0.3, cy: -0.4,fx: 0.3, fy: -0.4,radius: 1.35, stop: 0 #012, stop: 1 #357);");
    //else                        this->setStyleSheet("color: #fff;border: none;border-radius: 5px;padding: 10px;background: qradialgradient(cx: 0.3, cy: -0.4,fx: 0.3, fy: -0.4,radius: 1.35, stop: 0 #357, stop: 1 #012);");
    repaint();
}
void QCustomButton::resetView()
{

  //  this->setStyleSheet("background-color: rgb(41, 84, 118);color:rgb(255, 255, 255);font:  12pt \"MS Shell Dlg 2\";border: none;border-radius: 5px;padding: 5px; ");
    this->setStyleSheet("background-color: rgb(41, 84, 118);"
                        "color:rgb(255, 255, 255);"
                        "border:none;"
                        "border-radius: 5px;"
                        "font:  12pt \"MS Shell Dlg 2\";"
                        );

    //if(QToolButton::isChecked())this->setStyleSheet("color: #fff;border: none;border-radius: 5px;padding: 10px;background: qradialgradient(cx: 0.3, cy: -0.4,fx: 0.3, fy: -0.4,radius: 1.35, stop: 0 #012, stop: 1 #357);");
    //else                        this->setStyleSheet("color: #fff;border: none;border-radius: 5px;padding: 10px;background: qradialgradient(cx: 0.3, cy: -0.4,fx: 0.3, fy: -0.4,radius: 1.35, stop: 0 #357, stop: 1 #012);");
    //this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    repaint();

}
bool QCustomButton::event(QEvent *event)
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
