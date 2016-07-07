#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include <QImage>
#include "Tracker.h"

namespace Ui {
class FrameView;
}

class FrameView : public QWidget
{
    Q_OBJECT

public:
    explicit FrameView(QWidget *parent = 0);
    ~FrameView();    

private:
    Ui::FrameView *ui;
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
private slots:
    void ShowVideoCam();
};

#endif // FRAMEVIEW_H
