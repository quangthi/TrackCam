#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QImage>
#include "Tracker.h"
#include "Config.h"
#include "videowork.h"


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

    CConfig         m_Config;
    CvVideoWriter*	m_Writer;
    RECT            m_rectCurrent;
    //std::string     m_strVideoFile;
    short int       m_centerX;
    short int       m_centerY;

    QThread         *m_thread;
    VideoWork       *m_worker;
    std::string     m_strVideoFile;


public:
    explicit VideoDisplay(QWidget *parent = 0);
    ~VideoDisplay();    
    void InitTimer();
    void resetPaint();

private:
    Ui::VideoDisplay *ui;




protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);
private slots:    
    void OnTimerDrawImage();

};

#endif // VIDEODISPLAY_H
