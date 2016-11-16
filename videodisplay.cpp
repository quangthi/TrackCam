#include "videodisplay.h"
#include "ui_videodisplay.h"

RECT trackingRect;

QImage                      *_qImg = NULL;
QTimer                      *_drawTimer;
IplImage                    *_bufferFrame = NULL;

QRect                       _toBeTracked;
bool                        _isSelecting = false;

int                         _nCaptureTimeOut = 0;

// for putting text to frame
CvFont      cvTxtFont;
double      hScale = 0.5;
double      vScale = 0.5;
double      lineWidth = 1.5;
std::string szTime, szDay;
char        szTmp[255];



VideoDisplay::VideoDisplay(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoDisplay)

{
    ui->setupUi(this);
    //this->move(m_Config._config.frmPosX, m_Config._config.frmPosY);
    //this->setFixedSize(m_Config._config.frmWidth, m_Config._config.frmHeight);
    //this->showFullScreen();
    //this->showMaximized();
    this->setCursor(Qt::CrossCursor);
    m_Writer = NULL;


    m_rectCurrent.left = 0;
    m_rectCurrent.top = 0;
    m_rectCurrent.right = 0;
    m_rectCurrent.bottom = 0;
    m_centerX   = 0;
    m_centerY   = 0;
    m_Zoom      = 1;
    m_Focus     = 0;
    m_Azi       = 0;
    m_Ele       = 0;
    m_StringView =QString::fromUtf8("Màn hình bám đối tượng - Viện Tích hợp hệ thống");

    _drawTimer = new QTimer();
    connect(_drawTimer, SIGNAL(timeout()), this, SLOT(OnTimerDrawImage()));

    // The thread and the worker are created in the constructor so it is always safe to delete them.
    m_thread = new QThread();
    m_worker = new VideoWork();

    m_worker->moveToThread(m_thread);
    connect(m_worker, SIGNAL(workRequested()), m_thread, SLOT(start()));
    connect(m_thread, SIGNAL(started()), m_worker, SLOT(doWork()));
    connect(m_worker, SIGNAL(finished()), m_thread, SLOT(quit()), Qt::DirectConnection);
}

VideoDisplay::~VideoDisplay()
{
    cvReleaseImage(&_bufferFrame);
    _bufferFrame = NULL;

    m_worker->abort();
    m_thread->wait();

    _drawTimer->stop();

    delete m_thread;
    delete m_worker;

    delete ui;
}

QString GetTimeString()
{
    return QTime::currentTime().toString(Qt::TextDate);
}

QString GetDateString()
{
    return QDate::currentDate().toString("dd.MM.yyyy");
}

uchar *qImageBuffer = NULL;
QImage *IplImageToQImage(const IplImage * iplImage, double mini, double maxi)
{
    if (!iplImage)
        return NULL;

    int width = iplImage->width;
    int widthStep = iplImage->widthStep;
    int height = iplImage->height;
    QImage *qImage;
    switch (iplImage->depth)
    {
        case IPL_DEPTH_8U:
        if (iplImage->nChannels == 1)
        {
        // OpenCV image is stored with one byte grey pixel. We convert it
       // to an 8 bit depth QImage.
        qImage = new QImage(width,height,QImage::Format_Indexed8);
        uchar *QImagePtr = qImage->scanLine(0);
        qImageBuffer = qImage->scanLine(0);

        const uchar *iplImagePtr = (const uchar *) iplImage->imageData;

        for (int y = 0; y < height; y++)
        {
            // Copy line by line
            QImagePtr = qImage->scanLine(y);
            memcpy(QImagePtr, iplImagePtr, width);
            iplImagePtr += widthStep;

        }
        /*
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                // We take only the highest part of the 16 bit value. It is
                //similar to dividing by 256.
                //*QImagePtr++ = ((*iplImagePtr++) >> 8);
                *QImagePtr = *iplImagePtr;
                QImagePtr++;
                iplImagePtr++;
                }

                iplImagePtr += widthStep/sizeof(uchar)-width;
            }*/
        }
        else if (iplImage->nChannels == 3)
            {
            /* OpenCV image is stored with 3 byte color pixels (3 channels).
            We convert it to a 32 bit depth QImage.
            */
            qImageBuffer = (uchar *) malloc(width*height*4*sizeof(uchar));
            uchar *QImagePtr = qImageBuffer;
            const uchar *iplImagePtr = (const uchar *) iplImage->imageData;
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    // We cannot help but copy manually.
                    QImagePtr[0] = iplImagePtr[0];
                    QImagePtr[1] = iplImagePtr[1];
                    QImagePtr[2] = iplImagePtr[2];
                    QImagePtr[3] = 0;

                    QImagePtr += 4;
                    iplImagePtr += 3;
                }
            iplImagePtr += widthStep-3*width;
            }

        }
        else
        {
            qDebug("IplImageToQImage: image format is not supported : depth=8U and %d channels\n", iplImage->nChannels);
        }
        break;
        case IPL_DEPTH_16U:
        if (iplImage->nChannels == 1)
        {
        /* OpenCV image is stored with 2 bytes grey pixel. We convert it
        to an 8 bit depth QImage.
        */
            qImage = new QImage(width,height,QImage::Format_Indexed8);
            uchar *QImagePtr = qImage->scanLine(0);
            qImageBuffer = qImage->scanLine(0);

            //const uint16_t *iplImagePtr = (const uint16_t *);
            const unsigned short *iplImagePtr = (const unsigned short *)iplImage->imageData;


            for (int y = 0; y < height; y++)
            {
                QImagePtr = qImage->scanLine(y);
                for (int x = 0; x < width; x++)
                {
                // We take only the highest part of the 16 bit value. It is
                //similar to dividing by 256.
                //*QImagePtr++ = ((*iplImagePtr++) >> 8);
                //change here 16 bit could be everything !! set max min to your desire
                *QImagePtr = 255*(((*iplImagePtr) - mini) / (maxi - mini));
                QImagePtr++;
                iplImagePtr++;
                }

                iplImagePtr += widthStep/sizeof(unsigned short)-width;
            }

        }
        else
        {
            qDebug("IplImageToQImage: image format is not supported : depth=16U and %d channels\n", iplImage->nChannels);

        }
        break;
        case IPL_DEPTH_32F:
         if (iplImage->nChannels == 1)
         {
        /* OpenCV image is stored with float (4 bytes) grey pixel. We
        convert it to an 8 bit depth QImage.
        */
             qImage = new QImage(width,height,QImage::Format_Indexed8);
             uchar *QImagePtr = qImage->scanLine(0);
             qImageBuffer = qImage->scanLine(0);

             const float *iplImagePtr = (const float *) iplImage->imageData;
             for (int y = 0; y < height; y++)
             {
             QImagePtr = qImage->scanLine(y);
                 for (int x = 0; x < width; x++)
                 {
                     uchar p;
                     float pf = 255 * ((*iplImagePtr++) - mini) / (maxi - mini);
                     if (pf < 0) p = 0;
                     else if (pf > 255) p = 255;
                     else p = (uchar) pf;

                     *QImagePtr++ = p;
                  }

             iplImagePtr += widthStep/sizeof(float)-width;
             }
         }
         else
         {
             qDebug("IplImageToQImage: image format is not supported : depth=32F and %d channels\n", iplImage->nChannels);
         }
       break;
       case IPL_DEPTH_64F:
         if (iplImage->nChannels == 1)
         {
            /* OpenCV image is stored with double (8 bytes) grey pixel. We
            convert it to an 8 bit depth QImage.
            */
             qImage = new QImage(width,height,QImage::Format_Indexed8);
             uchar *QImagePtr = qImage->scanLine(0);
             qImageBuffer = qImage->scanLine(0);

            const double *iplImagePtr = (const double *) iplImage->imageData;
            for (int y = 0; y < height; y++)
            {
             QImagePtr = qImage->scanLine(y);
                for (int x = 0; x < width; x++)
                {
                    uchar p;
                    double pf = 255 * ((*iplImagePtr++) - mini) / (maxi - mini);

                    if (pf < 0) p = 0;
                    else if (pf > 255) p = 255;
                    else p = (uchar) pf;

                    *QImagePtr++ = p;
                }

            }

        }
        else
        {
            qDebug("IplImageToQImage: image format is not supported : depth=64F and %d channels\n", iplImage->nChannels);
        }
        break;
        default:
        qDebug("IplImageToQImage: image format is not supported : depth=%d and %d channels\n", iplImage->depth, iplImage->nChannels);
    }

    QVector<QRgb> vcolorTable;
    if (iplImage->nChannels == 1)
    {
        // We should check who is going to destroy this allocation.
        vcolorTable.resize(256);
        for (int i = 0; i < 256; i++)
        {
           vcolorTable[i] = qRgb(i, i, i);
        }
        //Qt vector is difficult to use... start with std to qvector
        //here I allocate QImage using qt constructor (Forma_Indexed8 adds sometimes 2 bytes on the right side !!! o.O not specified nowhere !!!)
        //qImage = new QImage(tmpImg->scanLine(0), width, height, QImage::Format_Indexed8);
        qImage->setColorTable(vcolorTable);
    }
    else
    {
        //if(qImage)delete qImage;
        qImage = new QImage(qImageBuffer, width, height, QImage::Format_RGB32);

    }
    //*data = qImageBuffer;
    //delete qImageBuffer;
    return qImage;
}

void VideoDisplay::InitTimer()
{
    _drawTimer->start(40);
}

void VideoDisplay::resetPaint()
{
    repaint();
}

void VideoDisplay::ResetRectCurrent()
{
    m_rectCurrent.left = 0;
    m_rectCurrent.top = 0;
    m_rectCurrent.right = 0;
    m_rectCurrent.bottom = 0;
}


void VideoDisplay::paintEvent(QPaintEvent *event)
{

    QPainter p(this);

    if (!m_worker->m_pFrame)
    {
        QFont font;
        font.setPointSize(30);
        p.setFont(font);
        p.setPen(QPen(Qt::red, 4));
        if (m_worker->m_IsCapturing)
            p.drawText(m_Config._config.frmWidth/2 - 100,
                       m_Config._config.frmHeight /2 +5, "Connecting...");
        else
            p.drawText(m_Config._config.frmWidth/2 - 80,
                       m_Config._config.frmHeight /2 +5, "NO VIDEO");
        return;
    }


    //qDebug()<<"Test";
    if(qImageBuffer)
        delete qImageBuffer;


    if (_bufferFrame)
        _qImg = IplImageToQImage(_bufferFrame,0,255);

    if (_qImg)
    {
        QRect rect(0, 0, m_Config._config.frmWidth, m_Config._config.frmHeight);
        p.drawRect(rect);
        p.drawImage(rect,*_qImg,_qImg->rect());

        QFont font;
        font.setPointSize(11);
        p.setFont(font);

        p.setPen(QPen(QColor(0,255,255), 2));

//        for (int i = 1; i <= m_StringListView.count(); i++)
//        {
//            p.drawText(6, i*18, m_StringListView.at(i-1));
//        }

        QString tmpStr ="Zoom : ";
        tmpStr += QString::number(m_Zoom/100.0f);
        tmpStr += "x";
        p.drawText(6, 16,tmpStr);

        tmpStr ="Focus : ";
        tmpStr += QString::number(m_Focus);
        p.drawText(5, 34,tmpStr);

        tmpStr =QString::fromUtf8("Ph.vị  : ");
        tmpStr += QString::number(m_Azi/100.0f);
        p.drawText(7, 52,tmpStr);


        tmpStr =QString::fromUtf8("Góc tà: ");
        tmpStr += QString::number(m_Ele/100.0f);
        p.drawText(5, 70,tmpStr);

        if (m_Writer)
        {
            p.setPen(QPen(Qt::red, 2));
            p.drawText(m_Config._config.frmWidth - 85, 18, "Recording...");
        }
    }

    if (!m_worker->m_IsTracking && !m_IsMouseOn)
    {        
        //QRect rect1(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2, m_rectWidthInit, m_rectHeightInit);
        p.setPen(QPen(Qt::red, 2));
        //p.setBrush(QBrush(Qt::transparent));

        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 - m_rectWidthInit / 2 + 10,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 - m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2 + 10);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 + m_rectWidthInit / 2 - 10,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 + m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 - m_rectHeightInit / 2 + 10);
        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 - m_rectWidthInit / 2 + 10,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 - m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2 - 10);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 + m_rectWidthInit / 2 - 10,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2,
                   m_Config._config.frmWidth / 2 + m_rectWidthInit / 2,
                   m_Config._config.frmHeight / 2 + m_rectHeightInit / 2 - 10);

    }

    else if (_isSelecting)
    {
        p.setPen(QPen(Qt::red, 2));
        p.setBrush(QBrush(Qt::transparent));
        p.drawRect(_toBeTracked);

    }

    else if (m_worker->m_IsTracking)
    {
        if ((m_rectCurrent.top == 0) && (m_rectCurrent.bottom == 0))
            m_rectCurrent = m_worker->m_rectCurrent;
        else
        {
            m_rectCurrent.top = (int)(0.3f*m_worker->m_rectCurrent.top + 0.7f*m_rectCurrent.top);
            m_rectCurrent.bottom = (int)(0.3f*m_worker->m_rectCurrent.bottom + 0.7f*m_rectCurrent.bottom);
            m_rectCurrent.left = (int)(0.3f*m_worker->m_rectCurrent.left + 0.7f*m_rectCurrent.left);
            m_rectCurrent.right = (int)(0.3f*m_worker->m_rectCurrent.right + 0.7f*m_rectCurrent.right);
        }
        p.setPen(QPen(QColor(0,255,0), 2));

        QPoint lefttop[3] = {
            QPoint(m_rectCurrent.left, m_rectCurrent.top + 10),
            QPoint(m_rectCurrent.left, m_rectCurrent.top),
            QPoint(m_rectCurrent.left + 10, m_rectCurrent.top),
        };
        QPoint righttop[3] = {
            QPoint(m_rectCurrent.right, m_rectCurrent.top + 10),
            QPoint(m_rectCurrent.right, m_rectCurrent.top),
            QPoint(m_rectCurrent.right - 10, m_rectCurrent.top),
        };
        QPoint rightbottom[3] = {
            QPoint(m_rectCurrent.right, m_rectCurrent.bottom - 10),
            QPoint(m_rectCurrent.right, m_rectCurrent.bottom),
            QPoint(m_rectCurrent.right - 10, m_rectCurrent.bottom),
        };
        QPoint leftbottom[3] = {
            QPoint(m_rectCurrent.left, m_rectCurrent.bottom - 10),
            QPoint(m_rectCurrent.left, m_rectCurrent.bottom),
            QPoint(m_rectCurrent.left + 10, m_rectCurrent.bottom),
        };
        p.drawPolyline(lefttop, 3);
        p.drawPolyline(righttop, 3);
        p.drawPolyline(rightbottom, 3);
        p.drawPolyline(leftbottom, 3);
    }


}


void VideoDisplay::mousePressEvent(QMouseEvent *event)
{

    if (!m_worker->m_pFrame)
        return;

    if (!m_IsMouseOn)
    {
        m_IsMouseOn = true;
    }

        //return;

    if(event->buttons() & Qt::LeftButton)
    {
        QRect videoRect(0, 0, m_Config._config.frmWidth, m_Config._config.frmHeight);

        if(videoRect.contains(event->x(),event->y()))
        {
            m_worker->m_IsTracking = false;            
            _isSelecting = true;
            trackingRect.left = event->x() - videoRect.left();
            trackingRect.top = event->y() - videoRect.top();

            _toBeTracked.setLeft(event->x());
            _toBeTracked.setTop(event->y());
            _toBeTracked.setRight(event->x()) ;
            _toBeTracked.setBottom( event->y());
        }
    }
    else if (event->buttons() & Qt::RightButton)
    {
        trackingRect.left		= event->x()-m_rectWidthInit / 2;
        trackingRect.top		= event->y()-m_rectHeightInit / 2;
        trackingRect.right		= event->x()+m_rectWidthInit / 2;
        trackingRect.bottom	= event->y()+m_rectHeightInit / 2;

        if(trackingRect.left < 0)
            trackingRect.left = 0;
        if(trackingRect.right > m_Config._config.frmWidth - 1)
            trackingRect.right = m_Config._config.frmWidth - 1;
        if(trackingRect.top < 0)
            trackingRect.top = 0;
        if(trackingRect.bottom > m_Config._config.frmHeight - 1)
            trackingRect.bottom = m_Config._config.frmHeight - 1;

        ResetRectCurrent();
        m_worker->StartTracking(trackingRect);
    }



    QMainWindow::mousePressEvent(event);
}

void VideoDisplay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_IsMouseOn)
        return;

    if (!m_worker->m_pFrame)
        return;

    if(_isSelecting)
    {
        _toBeTracked.setRight(event->x()) ;
        _toBeTracked.setBottom(event->y());
        if(!_toBeTracked.contains(event->x(),event->y()))
            _isSelecting = false;
    }
}

void VideoDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_IsMouseOn)
        return;

    if (!m_worker->m_pFrame)
        return;

    m_IsMouseOn = false;

    if(_isSelecting)
    {
        QRect videoRect(0, 0, m_Config._config.frmWidth, m_Config._config.frmHeight);
        trackingRect.right = event->x() - videoRect.left();
        trackingRect.bottom = event->y() - videoRect.top();
        short temp;
        if(trackingRect.right<trackingRect.left)
        {
            temp = trackingRect.right;
            trackingRect.right  = trackingRect.left;
            trackingRect.left = temp;
        }
        if(trackingRect.bottom<trackingRect.top)
        {
            temp = trackingRect.bottom;
            trackingRect.bottom = trackingRect.top;
            trackingRect.top = temp;
        }
        if(trackingRect.left < 0)
            trackingRect.left = 0;
        if(trackingRect.right > m_Config._config.frmWidth - 1)
            trackingRect.right = m_Config._config.frmWidth - 1;
        if(trackingRect.top < 0)
            trackingRect.top = 0;
        if(trackingRect.bottom > m_Config._config.frmHeight - 1)
            trackingRect.bottom = m_Config._config.frmHeight - 1;

        ResetRectCurrent();
        m_worker->StartTracking(trackingRect);
        _isSelecting = false;        
    }

    QMainWindow::mouseReleaseEvent(event);
}

void VideoDisplay::OnTimerDrawImage()
{
    if (m_worker->m_IsCapturing)
    {
        _nCaptureTimeOut++;
        repaint();
    }

    else
        _nCaptureTimeOut = 0;

    if (_nCaptureTimeOut >= 500) // timeout 20 seconds
    {
        QApplication::quit();
//        _nCaptureTimeOut = 0;

//        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "TrackCam",
//                                                                    tr("Video not found!\n Exit Application?\n"),
//                                                                    QMessageBox::No | QMessageBox::Yes,
//                                                                    QMessageBox::Yes);
//        if (resBtn == QMessageBox::Yes)
//        {
//            QApplication::quit();
//        }
    }


    if (!m_worker->m_pFrame)
    {
        repaint();
        return;
    }


    if (!_bufferFrame)
        _bufferFrame = cvCreateImage(cvSize(m_Config._config.frmWidth, m_Config._config.frmHeight), 8, 3);

    cvCopy(m_worker->m_pFrame, _bufferFrame);

    if (m_worker->m_IsTracking)
    {
        CvRect	nCvRectBox = cvRect(0, 0, 0, 0);
        utl_ConvertRectToBox(m_worker->m_rectCurrent, &nCvRectBox);
        m_centerX = ((nCvRectBox.x + nCvRectBox.width / 2)
                    - (m_Config._config.frmWidth / 2)) * 100 / m_Config._config.frmWidth;
        m_centerY = ((nCvRectBox.y + nCvRectBox.height / 2)
                    - (m_Config._config.frmHeight / 2)) * 100 / m_Config._config.frmHeight;
    }
    else
    {
        m_centerX = 0;
        m_centerY = 0;
    }



    cvInitFont(&cvTxtFont, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hScale, vScale, 0, lineWidth);

    if (m_strVideoFile == "")
    {
        szTime  = GetTimeString().toStdString();
        fn_ConvStrChar(szTime, szTmp);
        cvPutText(_bufferFrame, szTmp, cvPoint(3, m_Config._config.frmHeight - 15), &cvTxtFont, cvScalar(255, 255, 255));
    }

    repaint();


    if (m_Writer != NULL)
    {
        szDay   = GetDateString().toStdString();
        fn_ConvStrChar(szDay, szTmp);
        cvPutText(_bufferFrame, szTmp, cvPoint(m_Config._config.frmWidth - 100, 15),
                  &cvTxtFont, cvScalar(255, 255, 255));        

        cvWriteToAVI(m_Writer, _bufferFrame);
    }

}


void VideoDisplay::closeEvent (QCloseEvent *event)
{
    event->ignore();
}
