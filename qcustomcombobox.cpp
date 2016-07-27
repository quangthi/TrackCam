#include "QCustomComboBox.h"

QCustomComboBox::QCustomComboBox(QWidget *parent) : QComboBox(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomComboBox::hoverEnter(QHoverEvent *)
{

    highLight();
}

void QCustomComboBox::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QCustomComboBox::hoverMove(QHoverEvent *)
{
    highLight();

}
void QCustomComboBox::highLight()
{
    this->setStyleSheet("background-color: rgb(41, 84, 118);"
                        "color:rgb(255, 255, 255);"
                        "font: bold 12pt \"MS Shell Dlg 2\";"
                        "border-radius: 5px;"
                        "border-color: beige;"
                        "border-style: outset;"
                         "border-width: 1px;");
    //this->setStyleSheet("background-color: rgb(27, 168, 184);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";");
    repaint();
}
void QCustomComboBox::resetView()
{
    this->setStyleSheet("background-color: rgb(41, 84, 118);"
                        "color:rgb(255, 255, 255);"
                        "border:none;"
                        "border-radius: 5px;"
                        "font:  12pt \"MS Shell Dlg 2\";"
                        );
    //this->setStyleSheet("background-color: rgb(41, 84, 118);color:rgb(255, 255, 255);");
    //this->setStyleSheet("background-color: rgb(41, 84, 118);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    repaint();

}
bool QCustomComboBox::event(QEvent *event)
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
