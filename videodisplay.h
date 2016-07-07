#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QImage>
#include "Tracker.h"
#include "Config.h"


namespace Ui {
class VideoDisplay;
}

class VideoDisplay : public QMainWindow
{
    Q_OBJECT
public:    
    int             m_rectWidthInit;
    int             m_rectHeightInit;
    bool            m_IsMouseOn;
    bool            m_IsTracking;

    CConfig         m_Config;
    CvVideoWriter*	m_Writer;
    RECT            m_rectCurrent;


public:
    explicit VideoDisplay(QWidget *parent = 0);
    ~VideoDisplay();

    void StartTracking(RECT inputRECT);
    void StopTracking();

private:
    Ui::VideoDisplay *ui;
    void InitTimer();


protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);
private slots:
    void ShowVideoCam();

    void OnTimerDraw();
};

#endif // VIDEODISPLAY_H
