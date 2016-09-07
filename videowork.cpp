#include "videowork.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

CvCapture                   *_gCapture = NULL;
IplImage                    *_gTrueFrame = NULL;

IplImage                    *_gFrameHalf = NULL;
CTracker                    _gTracker;


VideoWork::VideoWork(QObject *parent) : QObject(parent)
{
    m_working =false;
    m_abort = false;
    m_pFrame = NULL;
    m_IsTracking = false;
    m_rectCurrent.left = 0;
    m_rectCurrent.top = 0;
    m_rectCurrent.right = 0;
    m_rectCurrent.bottom = 0;
    m_IsCapturing = false;
}

void VideoWork::requestWork()
{
    m_mutex.lock();
    m_working = true;
    m_abort = false;
    m_IsTracking = false;
    qDebug()<<"Request worker start in Thread "<<thread()->currentThreadId();
    m_mutex.unlock();

    emit workRequested();
}

void VideoWork::abort()
{
    m_mutex.lock();
    if (m_working) {
        m_abort = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
    m_mutex.unlock();
}

void VideoWork::doWork()
{
    m_IsCapturing = true;
    if (m_strVideoFile != "")
    {
        char		szTmp[255];
        fn_ConvStrChar(m_strVideoFile, szTmp);

        _gCapture = cvCaptureFromFile(szTmp);        
    }
    else if (m_Config._config.ipCam == 1)
    {
        char szTmp[255];
        fn_ConvStrChar(m_Config._config.strCamUrl, szTmp);
        _gCapture = cvCaptureFromFile(szTmp);
    }
    else if (m_Config._config.ipCam == 2)
    {
        char szTmp[255];
        fn_ConvStrChar(m_Config._config.strCamUrl_ir, szTmp);
        _gCapture = cvCaptureFromFile(szTmp);
    }
    else if (m_Config._config.ipCam == 0)
        _gCapture = cvCaptureFromCAM(0);

    m_IsCapturing = false;

    if (!_gCapture)
    {
//        QMessageBox msgBox;
//        msgBox.setText("Video not found!");
//        msgBox.exec();

        cvReleaseImage(&m_pFrame);
        m_pFrame = NULL;
        cvReleaseCapture(&_gCapture);
        _gCapture = NULL;

        // Set _working to false, meaning the process can't be aborted anymore.
        m_mutex.lock();
        m_working = false;
        m_mutex.unlock();

        emit finished();
    }



    if (m_pFrame == NULL)
        m_pFrame = cvCreateImage(cvSize(m_Config._config.frmWidth, m_Config._config.frmHeight), 8, 3);

    CvRect cvRectBox = cvRect(0, 0, 0, 0);

    while(true)
    {
        // Checks if the process should be aborted
        m_mutex.lock();
        bool abort = m_abort;
        m_mutex.unlock();        

        if (abort)
        {
            qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
            break;
        }

        try
        {
            _gTrueFrame = cvQueryFrame(_gCapture);
           // phan code duoc bao ve
        }catch(...)
        {
            continue;
          // phan code de xu ly bat ky kieu ngoai le nao
        }




//        if (_gTrueFrame)
//        {
//            int _width = _gTrueFrame->width;
//            int _height = _gTrueFrame->height;


//            qDebug() << "width:" << QString::number(_width);
//            qDebug() << "height:" << QString::number(_height);
//        }

        m_mutex.lock();
        if(!_gTrueFrame )
        {
            //_gCapture = NULL;
            //cvReleaseImage(&m_pFrame);
            //m_pFrame = NULL;
            m_IsTracking = false;
            m_mutex.unlock();
            continue;
        }
        else if (_gTrueFrame)
        {
            cvResize(_gTrueFrame, m_pFrame);
        }

        bool isTracking = m_IsTracking;
        m_mutex.unlock();


        if (isTracking) // if tracking opt is ON
        {
            cvResize(_gTrueFrame, _gFrameHalf, CV_INTER_LINEAR); // g_FrameHalf is initial when StartTracking function is called
            _gTracker.TrackNextFrame(_gFrameHalf, _gTracker.gRectCurrentHalf, &_gTracker.m_TrkResult);
            _gTracker.gRectCurrentHalf = _gTracker.m_TrkResult.targetBox;
            utl_RectCheckBound(&_gTracker.gRectCurrentHalf, _gTracker.m_ImageMaxX/2, _gTracker.m_ImageMaxY/2);

            m_mutex.lock();
            //update full size RECT for display
            _gTracker.m_RectCurrent.left	= _gTracker.gRectCurrentHalf.left*2;
            _gTracker.m_RectCurrent.top	= _gTracker.gRectCurrentHalf.top*2;
            _gTracker.m_RectCurrent.right	= _gTracker.gRectCurrentHalf.right*2;
            _gTracker.m_RectCurrent.bottom	= _gTracker.gRectCurrentHalf.bottom*2;

            m_rectCurrent = _gTracker.m_RectCurrent;


            utl_ConvertRectToBox(m_rectCurrent, &cvRectBox);
            if ((cvRectBox.width <= 10) ||(cvRectBox.height <= 10))
                StopTracking();

            m_mutex.unlock();
        }

        if (m_strVideoFile != "")
        {
            QEventLoop loop;
            QTimer::singleShot(40, &loop, SLOT(quit()));
            loop.exec();
        }

    }


     cvReleaseImage(&m_pFrame);
     m_pFrame = NULL;
     cvReleaseImage(&_gFrameHalf);
     _gFrameHalf = NULL;
     cvReleaseCapture(&_gCapture);
     _gCapture = NULL;

     // Set _working to false, meaning the process can't be aborted anymore.
     m_mutex.lock();
     m_working = false;
     m_mutex.unlock();

     emit finished();
}

void VideoWork::StartTracking(RECT inputRECT)
{
    if (!m_pFrame)
        return;

    m_mutex.lock();

    _gTracker.InitForFirstFrame1(m_pFrame, inputRECT);
    _gFrameHalf = cvCreateImage(cvSize(_gTracker.m_ImageMaxX/2, _gTracker.m_ImageMaxY/2), 8, 3);
    m_IsTracking = true;

    m_mutex.unlock();

}

void VideoWork::StopTracking()
{
    m_IsTracking = false;    
}
