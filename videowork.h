#ifndef VIDEOWORK_H
#define VIDEOWORK_H

#include <QObject>
#include <QMutex>
#include <QMessageBox>
#include "Tracker.h"
#include "Config.h"

class VideoWork : public QObject
{
    Q_OBJECT
public:
    CConfig         m_Config;
    RECT            m_rectCurrent;
    IplImage        *m_pFrame;
    bool            m_IsTracking;
    std::string     m_strVideoFile;
    bool            m_IsCapturing;
public:
    explicit VideoWork(QObject *parent = 0);
    void requestWork();
    void abort();
    void StartTracking(RECT inputRECT);
    void StopTracking();    

private:
    bool    m_abort;
    bool    m_working;
    QMutex  m_mutex;

signals:
    void workRequested();    
    void finished();

public slots:
    void doWork();
};

#endif // VIDEOWORK_H
