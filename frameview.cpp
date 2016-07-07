#include "frameview.h"
#include "ui_frameview.h"


RECT trackingRect;

QImage *img = NULL;
QTimer *frameTimer;
CvCapture                   *gCapture = NULL;
IplImage                    *gTrueFrame = NULL;
IplImage                    *gFrame = NULL;
IplImage                    *gFrameHalf = NULL;
bool                        gIsTracking = false;
CTracker                    gTracker;

QRect toBeTracked;
bool isSelecting = false;

static short                mousePointerX,mousePointerY;

FrameView::FrameView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrameView)
{
    ui->setupUi(this);
    this->move(0, 0);
    frameTimer  = new QTimer();
    connect(frameTimer, SIGNAL(timeout()), this, SLOT(ShowVideoCam()));
    frameTimer->start(30);
}

FrameView::~FrameView()
{
    frameTimer->stop();
    cvReleaseCapture(&gCapture);
    delete ui;
}

void inline DrawTrackingRgn(IplImage* img, RECT rect)
{
    if (!gIsTracking)
        return;

    CvRect cvRectBox = cvRect(0, 0, 0, 0);

    utl_ConvertRectToBox(rect, &cvRectBox);

    if ((cvRectBox.width <= 10) ||(cvRectBox.height <= 10))
    {
        gIsTracking = false;
        return;
    }

    CvPoint ltPoint = cvPoint(cvRectBox.x, cvRectBox.y);                                        // left top point of region
    CvPoint bdPoint = cvPoint(cvRectBox.x + cvRectBox.width, cvRectBox.y + cvRectBox.height);	// bottom down point of region
    CvPoint rtPoint = cvPoint(cvRectBox.x + cvRectBox.width, cvRectBox.y);                      // right top point of region
    CvPoint lbPoint = cvPoint(cvRectBox.x, cvRectBox.y + cvRectBox.height);                     // left bottom point of region

    CvScalar trackScalar = cvScalar(0, 255, 0);

    cvLine(img, ltPoint, cvPoint(ltPoint.x + 10, ltPoint.y), trackScalar, 2);
    cvLine(img, ltPoint, cvPoint(ltPoint.x, ltPoint.y + 10), trackScalar, 2);

    cvLine(img, bdPoint, cvPoint(bdPoint.x - 10, bdPoint.y), trackScalar, 2);
    cvLine(img, bdPoint, cvPoint(bdPoint.x, bdPoint.y - 10), trackScalar, 2);

    cvLine(img, rtPoint, cvPoint(rtPoint.x - 10, rtPoint.y), trackScalar, 2);
    cvLine(img, rtPoint, cvPoint(rtPoint.x, rtPoint.y + 10), trackScalar, 2);

    cvLine(img, lbPoint, cvPoint(lbPoint.x, lbPoint.y - 10), trackScalar, 2);
    cvLine(img, lbPoint, cvPoint(lbPoint.x + 10, lbPoint.y), trackScalar, 2);

    return;
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
    return qImage;
}

void FrameView::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    if (img)
    {
        QRect rect = this->geometry();
        p.drawRect(rect);
        p.drawImage(rect,*img,img->rect());
    }
}

void FrameView::mousePressEvent(QMouseEvent *event)
{
    if(event->x()>height())
    {
        if(event->buttons() & Qt::LeftButton)
        {
            QRect videoRect = this->geometry();

            if(videoRect.contains(event->x(),event->y()))
            {
                gIsTracking = false;
                isSelecting = true;
                trackingRect.left = event->x() - videoRect.left();
                trackingRect.top = event->y() - videoRect.top();

                toBeTracked.setLeft(event->x());
                toBeTracked.setTop(event->y());
                toBeTracked.setRight(event->x()) ;
                toBeTracked.setBottom( event->y());
            }
        }
        else
        {
            gIsTracking = false;
        }


    }
    else
    {
        mousePointerX = (event->x());
        mousePointerY = (event->y());

        //ui->frame_RadarViewOptions->hide();
        if(event->buttons() & Qt::LeftButton)
        {
            mousePointerX=event->x();
            mousePointerY=event->y();
            //printf("mouseX %d\n",mouseX);
        }
    }
    //QMainWindow::mousePressEvent(event);
}

void FrameView::ShowVideoCam()
{
    if (gCapture ==  NULL)
        gCapture = cvCaptureFromCAM(0);
    if (!gCapture)
    {
        frameTimer->stop();
        return;
    }

    if (gFrame == NULL)
        gFrame = cvCreateImage(cvSize(400, 300), 8, 3);

    gTrueFrame = cvQueryFrame(gCapture);

    if(!gTrueFrame)
        return;

    cvResize(gTrueFrame, gFrame);

    if (gIsTracking) // if tracking opt is ON
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
        DrawTrackingRgn(gFrame, gTracker.m_RectCurrent);
    }

    if(qImageBuffer)
        delete qImageBuffer;
    img = IplImageToQImage(gFrame,NULL,0,255);
    repaint();

}
