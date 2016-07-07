#include "videodisplay.h"
#include "ui_videodisplay.h"

RECT trackingRect;

QImage                      *qImg = NULL;
QTimer                      *frameTimer, *drawTimer;
CvCapture                   *gCapture = NULL;
IplImage                    *gTrueFrame = NULL;
IplImage                    *gFrame = NULL;
IplImage                    *gFrameHalf = NULL;
CTracker                    gTracker;


QRect toBeTracked;
bool isSelecting = false;

static short                mousePointerX,mousePointerY;

VideoDisplay::VideoDisplay(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoDisplay)

{
    ui->setupUi(this);
    this->move(m_Config._config.frmPosX, m_Config._config.frmPosY);
    this->setFixedSize(m_Config._config.frmWidth, m_Config._config.frmHeight);

    m_IsTracking = false;
    m_Writer = NULL;

    m_rectCurrent.left = 0;
    m_rectCurrent.top = 0;
    m_rectCurrent.right = 0;
    m_rectCurrent.bottom = 0;


    InitTimer();    
}

VideoDisplay::~VideoDisplay()
{    
    m_IsTracking = false;
    frameTimer->stop();
    drawTimer->stop();
    cvReleaseCapture(&gCapture);    

    delete ui;
}

int inline ConvStrChar(std::string szStr, char *szBuff)
{
    int	nLen = szStr.length();

    nLen = (nLen < 254)? nLen : 254;		// Get min(nLeng,nSize)	- 254 = max buff
    for (int i = 0; i < nLen; i ++)
        szBuff[i] = (char)szStr[i];

    szBuff[nLen] = 0x00;
    return nLen;
}


bool inline DrawTrackingRgn(IplImage* img, RECT rect)
{
//    if (!m_IsTracking)
//        return;


    CvRect cvRectBox = cvRect(0, 0, 0, 0);

    utl_ConvertRectToBox(rect, &cvRectBox);

    if ((cvRectBox.width <= 10) ||(cvRectBox.height <= 10))
    {
//        m_IsTracking = false;
        return false;
    }

    CvPoint ltPoint = cvPoint(cvRectBox.x, cvRectBox.y);                                        // left top point of region
    CvPoint bdPoint = cvPoint(cvRectBox.x + cvRectBox.width, cvRectBox.y + cvRectBox.height);	// bottom down point of region
    CvPoint rtPoint = cvPoint(cvRectBox.x + cvRectBox.width, cvRectBox.y);                      // right top point of region
    CvPoint lbPoint = cvPoint(cvRectBox.x, cvRectBox.y + cvRectBox.height);                     // left bottom point of region

    CvScalar trackScalar = cvScalar(0, 255, 0);

    cvLine(img, ltPoint, cvPoint(ltPoint.x + 10, ltPoint.y), trackScalar, 1);
    cvLine(img, ltPoint, cvPoint(ltPoint.x, ltPoint.y + 10), trackScalar, 1);

    cvLine(img, bdPoint, cvPoint(bdPoint.x - 10, bdPoint.y), trackScalar, 1);
    cvLine(img, bdPoint, cvPoint(bdPoint.x, bdPoint.y - 10), trackScalar, 1);

    cvLine(img, rtPoint, cvPoint(rtPoint.x - 10, rtPoint.y), trackScalar, 1);
    cvLine(img, rtPoint, cvPoint(rtPoint.x, rtPoint.y + 10), trackScalar, 1);

    cvLine(img, lbPoint, cvPoint(lbPoint.x, lbPoint.y - 10), trackScalar, 1);
    cvLine(img, lbPoint, cvPoint(lbPoint.x + 10, lbPoint.y), trackScalar, 1);

    return true;
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
QImage *IplImageToQImage(const IplImage * iplImage, uchar **data, double mini, double maxi)
{

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
    // Init Video Timer
    frameTimer  = new QTimer();
    connect(frameTimer, SIGNAL(timeout()), this, SLOT(ShowVideoCam()));
    frameTimer->start(20);

    drawTimer = new QTimer();
    connect(drawTimer, SIGNAL(timeout()), this, SLOT(OnTimerDraw()));
    drawTimer->start(40);
}


void VideoDisplay::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    if(qImageBuffer)
        delete qImageBuffer;

    if (gFrame)
        qImg = IplImageToQImage(gFrame,NULL,0,255);

    if (qImg)
    {
        QRect rect(0, 0, m_Config._config.frmWidth, m_Config._config.frmHeight);
        p.drawRect(rect);
        p.drawImage(rect,*qImg,qImg->rect());
    }

    if (!m_IsTracking && !m_IsMouseOn)
    {
        //QRect rect1(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2, m_rectWidthInit, m_rectHeightInit);
        p.setPen(QPen(Qt::red));
        p.setBrush(QBrush(Qt::transparent));

        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2, m_Config._config.frmWidth / 2 - m_rectWidthInit / 2 + 10, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2, m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2 + 10);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2, m_Config._config.frmWidth / 2 + m_rectWidthInit / 2 - 10, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2, m_Config._config.frmWidth / 2 + m_rectWidthInit / 2, m_Config._config.frmHeight / 2 - m_rectHeightInit / 2 + 10);
        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2, m_Config._config.frmWidth / 2 - m_rectWidthInit / 2 + 10, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2, m_Config._config.frmWidth / 2 - m_rectWidthInit / 2, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2 - 10);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2, m_Config._config.frmWidth / 2 + m_rectWidthInit / 2 - 10, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2);
        p.drawLine(m_Config._config.frmWidth / 2 + m_rectWidthInit / 2, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2, m_Config._config.frmWidth / 2 + m_rectWidthInit / 2, m_Config._config.frmHeight / 2 + m_rectHeightInit / 2 - 10);

    }
    else if (isSelecting)
    {
        p.setPen(QPen(Qt::red));
        p.setBrush(QBrush(Qt::transparent));
        p.drawRect(toBeTracked);
    }
}


void VideoDisplay::mousePressEvent(QMouseEvent *event)
{
    if (!m_IsMouseOn)
        return;

    if(event->buttons() & Qt::LeftButton)
    {
        QRect videoRect(0, 0, m_Config._config.frmWidth, m_Config._config.frmHeight);

        if(videoRect.contains(event->x(),event->y()))
        {
            m_IsTracking = false;
            isSelecting = true;
            trackingRect.left = event->x() - videoRect.left();
            trackingRect.top = event->y() - videoRect.top();

            toBeTracked.setLeft(event->x());
            toBeTracked.setTop(event->y());
            toBeTracked.setRight(event->x()) ;
            toBeTracked.setBottom( event->y());
        }
    }
    QMainWindow::mousePressEvent(event);
}

void VideoDisplay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_IsMouseOn)
        return;

    if(isSelecting)
    {
        toBeTracked.setRight(event->x()) ;
        toBeTracked.setBottom(event->y());
        if(!toBeTracked.contains(event->x(),event->y()))
            isSelecting = false;
    }
}

void VideoDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_IsMouseOn)
        return;

    if(isSelecting)
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

        StartTracking(trackingRect);
        isSelecting = false;
    }

    QMainWindow::mouseReleaseEvent(event);
}

void VideoDisplay::ShowVideoCam()
{
    if (gCapture ==  NULL)
    {
        if (m_Config._config.ipCam == 1)
        {
            char		szTmp[255];
            ConvStrChar(m_Config._config.strCamUrl, szTmp);
            gCapture = cvCaptureFromFile(szTmp);
        }
        else if (m_Config._config.ipCam == 0)
            gCapture = cvCaptureFromCAM(0);

    }

    if (!gCapture)
    {
        frameTimer->stop();
        drawTimer->stop();
        close();
        return;
    }

    if (gFrame == NULL)
        gFrame = cvCreateImage(cvSize(m_Config._config.frmWidth, m_Config._config.frmHeight), 8, 3);

    gTrueFrame = cvQueryFrame(gCapture);

    if(!gTrueFrame)
        return;

    cvResize(gTrueFrame, gFrame);

    if (m_IsTracking) // if tracking opt is ON
    {
        cvResize(gFrame, gFrameHalf, CV_INTER_LINEAR); // g_FrameHalf is initial when StartTracking function is called
        gTracker.TrackNextFrame(gFrameHalf, gTracker.gRectCurrentHalf, &gTracker.m_TrkResult);
        gTracker.gRectCurrentHalf = gTracker.m_TrkResult.targetBox;
        utl_RectCheckBound(&gTracker.gRectCurrentHalf, gTracker.m_ImageMaxX/2, gTracker.m_ImageMaxY/2);

        //update full size RECT for display
        gTracker.m_RectCurrent.left	= gTracker.gRectCurrentHalf.left*2;
        gTracker.m_RectCurrent.top	= gTracker.gRectCurrentHalf.top*2;
        gTracker.m_RectCurrent.right	= gTracker.gRectCurrentHalf.right*2;
        gTracker.m_RectCurrent.bottom	= gTracker.gRectCurrentHalf.bottom*2;
        if (!DrawTrackingRgn(gFrame, gTracker.m_RectCurrent))
        {
            StopTracking();
            return;
        }

        m_rectCurrent = gTracker.m_RectCurrent;
    }

    // save video to hard disk
//    if (m_Writer != NULL)
//        cvWriteToAVI(m_Writer, gFrame);


//    if(qImageBuffer)
//        delete qImageBuffer;
//    qImg = IplImageToQImage(gFrame,NULL,0,255);
//    repaint();

}



void VideoDisplay::StartTracking(RECT inputRECT)
{
    if (!gFrame)
        return;

    gTracker.InitForFirstFrame1(gFrame, inputRECT);
    gFrameHalf = cvCreateImage(cvSize(gTracker.m_ImageMaxX/2, gTracker.m_ImageMaxY/2), 8, 3);
    m_IsTracking = true;

}

void VideoDisplay::StopTracking()
{
    m_IsTracking = false;
    cvReleaseImage(&gFrameHalf);
}

CvFont      cvTxtFont;
double      hScale = 0.5;
double      vScale = 0.5;
int         lineWidth = 1;
std::string szTime, szDay;
char        szTmp[255];


void VideoDisplay::OnTimerDraw()
{
    repaint();
    if (m_Writer != NULL)
    {
        szTime  = GetTimeString().toStdString();
        szDay   = GetDateString().toStdString();
        cvInitFont(&cvTxtFont, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hScale, vScale, 0, lineWidth);
        ConvStrChar(szTime, szTmp);
        cvPutText(gFrame, szTmp, cvPoint(3, 15), &cvTxtFont, cvScalar(255, 0, 0));

        ConvStrChar(szDay, szTmp);
        cvPutText(gFrame, szTmp, cvPoint(m_Config._config.frmWidth - 110, 15), &cvTxtFont, cvScalar(255, 0, 0));

        cvWriteToAVI(m_Writer, gFrame);
    }

}


void VideoDisplay::closeEvent (QCloseEvent *event)
{
    event->ignore();
}
