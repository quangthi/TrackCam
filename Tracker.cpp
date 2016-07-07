#include "Tracker.h"

CTracker::CTracker()
{
    m_BoxRatio = 1.0;
    m_pImgCurrent=NULL;
    m_pImgCurrentHalf=NULL;
    gImgInitMaskHalf=NULL;
    gImgInitMask=NULL;
    m_ImageWidth=720;
    m_ImageHeight=480;

    m_TrkResult.FGImage = NULL;
    m_TrkResult.FGMask = NULL;
    m_TrkResult.ObjMask = NULL;

}

void CTracker::GenColorFeatures()
{
    int r, g, b;
    int featr, featg, featb;
    int tmpr, tmpg, tmpb;
    int	i, j, k, idx; //loop index
    int alphavals[5] = {0, 1, -1, 2, -2};
    int minval, maxval, sumval;
    int sumabs, sumneg;
    int okflag;
    int featNum;
    //	double denum;
    //	double uveclist[FEATNUM][3];
    char strLabel[10];
    char letters[3] = {'R','G', 'B'};
    int	 first;
    char strtmp[3];

    int	 featlist[FEATNUM][5];
    char  featlabels[FEATNUM][10];

    //get feature list
    featNum = 0;
    for(i=0;i<5;i++)
    {
        for(j=0;j<5;j++)
        {
            for(k=0;k<5;k++)
            {
                r = alphavals[i];
                g = alphavals[j];
                b = alphavals[k];
                if (r*r+g*g+b*b>0)	// ((r!=0)&&(g!=0)&&(b!=0))
                {
                    minval = utl_Min3(r,g,b);
                    maxval = utl_Max3(r,g,b);
                    sumval = r+g+b;
                    if (abs(minval) > abs(maxval))
                    {
                        r = -r;
                        g = -g;
                        b = -b;
                    }
                    else if(sumval<0)
                    {
                        r = -r;
                        g = -g;
                        b = -b;
                    }
                    okflag = 1;
                    //test if this feature parallel with exist features
                    for (idx=0;idx<featNum;idx++)
                    {
                        featr = featlist[idx][0];
                        featg = featlist[idx][1];
                        featb = featlist[idx][2];
                        tmpr = g*featb -b*featg;
                        tmpg = b*featr - r*featb;
                        tmpb = r*featg - g*featr;
                        if (tmpr*tmpr+tmpg*tmpg+tmpb*tmpb==0)
                        {
                            okflag = 0;
                            break;
                        }
                    }
                    if (okflag ==1)
                    {
                        sumabs = abs(r)+abs(g)+abs(b);
                        sumneg = 0;
                        if (r<0) sumneg = sumneg +r;
                        if (g<0) sumneg = sumneg +g;
                        if (b<0) sumneg = sumneg +b;

                        featlist[featNum][0] = r;
                        featlist[featNum][1] = g;
                        featlist[featNum][2] = b;
                        featlist[featNum][3] = -256*sumneg;
                        featlist[featNum][4] = sumabs;

                        //						denum = sqrt(r*r+g*g+b*b);
                        //						uveclist[featNum][0] = r/denum;
                        //						uveclist[featNum][1] = g/denum;
                        //						uveclist[featNum][2] = b/denum;
                        featNum = featNum+1;
                    }
                }
            }//end b
        }//end g
    }//end r

    for (i=0;i<featNum;i++)
    {
        strcpy_s(strLabel,"");
        first = 1;
        for (j=0;j<3;j++)
        {
            if (featlist[i][j]!=0)
            {
                if (!first && featlist[i][j]>0)
                {
                    strcat(strLabel, "+");
                }
                if (featlist[i][j]<0)
                {
                    strcat(strLabel, "-");
                }

                if (abs(featlist[i][j])==1)
                {

                    memset(&strtmp[0], 0x00, 3);
                    memcpy(strtmp, &letters[j], 1);
                    //wsprintf(strtmp, "%c", letters[j]);
                    strcat(strLabel, strtmp);
                }
                else
                {

                    char* tmpChar = new char[1];            // Tim cach toi uu doan code nay, rat anh hương den qua trinh tracking
                    itoa(abs(featlist[i][j]), tmpChar, 10);
                    memset(&strtmp[0], 0x00, 3);
                    memcpy(strtmp, &tmpChar[0], 1);
                    memcpy(strtmp + 1, &letters[j], 1);
                    //wsprintf(strtmp, "%d%c",  abs(featlist[i][j]),letters[j]);
                    strcat(strLabel, strtmp);
                }
                first = 0;
            }
        }
        strcpy_s(featlabels[i],strLabel);

    }

    //return to global variable
    for (i=0;i<featNum;i++)
    {
        for(j=0;j<5;j++)
        {
            m_Featlist[i][j] = featlist[i][j];
        }
        strcpy_s(m_Featlabels[i], featlabels[i]);
    }

}

void CTracker::GetImageWidthHeightXY(IplImage* imgInput)
{
    m_ImageWidth = imgInput->width;
    m_ImageHeight = imgInput->height;
    m_ImageMaxX = m_ImageWidth;
    m_ImageMaxY = m_ImageHeight;
}

float CTracker::CountFGPixel(IplImage *inImg, RECT inRect)
    //cout foreground pixel number to find score
{
    int pixCount;
    float score;
    CvRect crectRoi;

    //count foreground points
    crectRoi.x = inRect.left;
    crectRoi.y = inRect.top;
    crectRoi.width = inRect.right - inRect.left;
    crectRoi.height = inRect.bottom - inRect.top;
    cvSetImageROI(inImg, crectRoi);
    pixCount = cvCountNonZero(inImg);
    inImg->roi = NULL;

    score = (float)pixCount/(crectRoi.width*crectRoi.height);

    return score;
}

void CTracker::MeanShift(IplImage *imgInput, int xstart, int ystart, int hx, int hy, double eps, double *modex, double *modey)
{
    double	xcur = xstart;
    double	ycur = ystart;
    double	eps2 = eps*eps;
    double	dist2 = eps2+1;
    int		ntimes = 0;
    double	denom, sumx, sumy;
    int		dx, dy, col, row;
    double  incx, incy;
    double	pixelval;

    //loop until shift dist below eps or times greater than threshold
    while (dist2>eps2 && ntimes<30) {
        //loop around target candidate window
        denom = 0;
        sumx = 0;
        sumy = 0;
        for(dx=-hx;dx<=hx;dx++){
            for(dy=-hy;dy<=hy;dy++){
                col = (int)fn_Round(dx+xcur);
                row = (int)fn_Round(dy+ycur);
                //judge if pixel is out of boundary
                if (col<0 || col>=imgInput->width ||row<0||row>=imgInput->height){
                    continue;
                }
                pixelval = cvGetReal2D(imgInput, row, col);
                denom = denom + fn_Abs(pixelval);
                sumx = sumx + dx*pixelval;
                sumy = sumy + dy*pixelval;
            }
        }

        //if denom is zero, target out of boundary
        if (denom==0){
            *modex = xcur;
            *modey = ycur;
            return;
        }
        incx = sumx/denom;
        incy = sumy/denom;

        xcur = fn_Round(xcur+incx);
        ycur = fn_Round(ycur+incy);

        dist2 = incx*incx + incy*incy;
        ntimes++;
    }

    //return result
    *modex = xcur;
    *modey = ycur;
}

void CTracker::CreateRegionMask(RECT inRect, IplImage *outImgMask)
{
    BOOL bResult;
        int	 row, col;
        BYTE *maskPixel;

        //set mask
        cvSetZero(outImgMask);
        for (row = inRect.top; row <= inRect.bottom; row++)
        {
            for (col = inRect.left; col <= inRect.right; col++)
            {
                //set mask image pixel as foreground
                maskPixel = cvPtr2D(outImgMask, row, col, NULL);
                (*maskPixel) = 255;
            }
        }
}

void CTracker::GetInitMask()
{
    RECT	rectCurrent;
    RECT	rectCurrentHalf;

    rectCurrent.left = m_RectInit.left;
    rectCurrent.top = m_RectInit.top;
    rectCurrent.right = m_RectInit.right;
    rectCurrent.bottom = m_RectInit.bottom;

    rectCurrentHalf.left = m_RectInit.left / 2;
    rectCurrentHalf.top = m_RectInit.top / 2;
    rectCurrentHalf.right = m_RectInit.right / 2;
    rectCurrentHalf.bottom = m_RectInit.bottom / 2;

    CreateRegionMask(rectCurrent, gImgInitMask);
    CreateRegionMask(rectCurrentHalf, gImgInitMaskHalf);
}

void CTracker::TrackInit(IplImage* imgInput, IplImage* imgObjMask, RECT inTargetBox)
{
    RECT	bgBox;
    int		row,col;
    CvScalar pixel;
    int		boxHalfWid, boxHalfHgt;
    int		dmax;
    int		i,j;
    double  feat;
    int		featbin;
    long	histObj[FEATNUM][BINNUM], histBg[FEATNUM][BINNUM];
    double  Pobj[FEATNUM][BINNUM], Pbg[FEATNUM][BINNUM];
    //double	Ptotal[FEATNUM][BINNUM];
    double  logRatio[FEATNUM][BINNUM];
    long	countObj, countBg;
    float	boxRatio = m_BoxRatio;			//ratio between forground and background bounding box

    //cvSaveImage("img0001.bmp", imgInput);


    m_pRatioImg = cvCreateImage(cvSize(imgInput->width, imgInput->height),32,1); //float image
    m_pWeightImg = cvCreateImage(cvSize(imgInput->width, imgInput->height),8,1);
    m_pImgFgMask = cvCreateImage(cvGetSize(imgInput), 8, 1);
    m_pImgObjMask = cvCreateImage(cvGetSize(imgInput), 8, 3);

    //generate FEATNUM feature combination list
    GenColorFeatures();

    //get outter bg bounding box
    boxHalfWid = (int)fn_Round((inTargetBox.right - inTargetBox.left)/2);
    boxHalfHgt = (int)fn_Round((inTargetBox.bottom - inTargetBox.top)/2);
    dmax = std::max(boxHalfWid, boxHalfHgt);

    bgBox.left = inTargetBox.left - (int)fn_Round(dmax*boxRatio);
    bgBox.right = inTargetBox.right + (int)fn_Round(dmax*boxRatio);
    bgBox.top = inTargetBox.top - (int)fn_Round(dmax*boxRatio);
    bgBox.bottom = inTargetBox.bottom + (int)fn_Round(dmax*boxRatio);
    utl_RectCheckBound(&bgBox,imgInput->width, imgInput->height);

    //compcolorfeatures for foreground and background
    countObj = 0;
    countBg = 0;
    for(i=0;i<FEATNUM;i++)
    {
        for(j=0;j<BINNUM;j++)
        {
            histObj[i][j] = 0;
            histBg[i][j] = 0;
        }
    }

    for(col=bgBox.left+1;col<=bgBox.right;col++)
    {
        for(row=bgBox.top+1;row<=bgBox.bottom;row++)
        {
            pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
            //judge obj or background
            //if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom){
            //	if (cvGetReal2D(imgObjMask, row,col)>0){
            if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom && cvGetReal2D(imgObjMask, row,col)>0)
            {
                //obj pixel
                countObj++;
                //calculate feature for feature list
                for(i=0;i<FEATNUM;i++)
                {
                    feat = (int)floor((m_Featlist[i][0]*pixel.val[2]+m_Featlist[i][1]*pixel.val[1]+m_Featlist[i][2]*pixel.val[0]+m_Featlist[i][3])/m_Featlist[i][4]);
                    featbin = (int)floor(feat/8);
                    //object histogram
                    histObj[i][featbin]++;
                }
            }
            else
            {
                //background pixel
                countBg++;
                //calculate feature for feature list
                for(i=0;i<FEATNUM;i++)
                {
                    feat = (m_Featlist[i][0]*pixel.val[2]+m_Featlist[i][1]*pixel.val[1]+m_Featlist[i][2]*pixel.val[0]+m_Featlist[i][3])/m_Featlist[i][4];
                    featbin = (int)floor(feat/8);
                    //background histogram
                    histBg[i][featbin]++;
                }
            }
        }
    }

    //get Fg pixel count
    m_MaskFgRatio = (float)countObj/((inTargetBox.right-inTargetBox.left)*(inTargetBox.bottom-inTargetBox.top));

    //normalize histogram and calculate log ratio
    for (i=0;i<FEATNUM;i++)
    {//for each feature
        for(j=0;j<BINNUM;j++)
        {//for each histogram bin
            Pobj[i][j] = (double)histObj[i][j]/countObj;
            m_PobjFirst[i][j] = Pobj[i][j]; //record the histogram of first frame into global array
            Pbg[i][j] = (double)histBg[i][j]/countBg;
            logRatio[i][j] =  std::max((double)-7, std::min((double)7,log((Pobj[i][j]+0.001)/(Pbg[i][j]+0.001))));
        }
    }

    //get max peak diff index
    {
        int featIdx;
        int maxPeakDiffIdx;
        double ratio;
        //		CvPoint	peak;
        CvPoint secondPeak;
        double maxVal, maxVal2;
        int		convWidth, convHeight;
        double minratio, maxratio;
        double scale;
        double shift;
        CvRect roi;
        int	 peakDiff, maxPeakDiff;
        IplImage *imgWeight;
        IplImage *imgBuffer;
        POINT	objCenter;


        roi.x = bgBox.left;
        roi.y = bgBox.top;
        roi.width = bgBox.right-bgBox.left;
        roi.height = bgBox.bottom-bgBox.top;
        cvSetZero(m_pWeightImg);
        cvSetImageROI(imgInput, roi);
        cvSetImageROI(m_pRatioImg, roi);
        cvSetImageROI(m_pWeightImg, roi);
        cvSetImageROI(imgObjMask, roi);
        cvNot(imgObjMask, imgObjMask);

        //get obj center
        objCenter.x = roi.width/2;
        objCenter.y = roi.height/2;

        //loop through featurelist to get maximum peak diff
        maxPeakDiff = 0;
        maxPeakDiffIdx = 0;
        convWidth = ((int)((inTargetBox.right - inTargetBox.left)/2))*2+1;
        convHeight = ((int)((inTargetBox.bottom - inTargetBox.top)/2))*2+1;

        imgWeight = (IplImage*)cvClone(m_pWeightImg);
        imgBuffer = (IplImage*)cvClone(m_pWeightImg);
        for (featIdx=0;featIdx<FEATNUM;featIdx++){//for each feature
            //get ratio image
            cvSetZero(m_pRatioImg);
            for(col=0;col<roi.width;col++)
            {
                for(row=0;row<roi.height;row++)
                {
                    pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
                    //calculate feature
                    feat = (m_Featlist[featIdx][0]*pixel.val[2]+m_Featlist[featIdx][1]*pixel.val[1]+m_Featlist[featIdx][2]*pixel.val[0]+m_Featlist[featIdx][3])/m_Featlist[featIdx][4];
                    featbin = (int)floor(feat/8);
                    ratio = logRatio[featIdx][featbin];
                    //set weight image
                    pixel.val[0] = ratio;
                    cvSet2D(m_pRatioImg, row, col, pixel);
                }
            }

            //get weight image by normalizing ratio image
            cvSetZero(imgWeight);
            cvMinMaxLoc(m_pRatioImg, &minratio, &maxratio, NULL, NULL, 0);
            shift = 0;
            if (maxratio<=minratio)
            {
                scale = 1;
            }
            else
            {
                //scale = 255.0/(maxratio-minratio);
                //shift = -minratio*scale;
                scale = 255.0/(fn_Abs(maxratio));
            }
            cvConvertScale(m_pRatioImg, imgWeight, scale, shift);
            //cvSaveImage("c:\\CTracker\\imgWeight.bmp", imgWeight);

            //mask out obj hole + Gaussian twice
            //smooth with Gaussian
            cvSmooth(imgWeight, imgBuffer, CV_GAUSSIAN, convWidth, convHeight,0);
            //cvSaveImage("c:\\CTracker\\imgBuffer_conv.bmp", imgBuffer);

            //get peak location;
            //cvMinMaxLoc(imgBuffer, NULL, &maxVal, NULL, &peak, NULL);
            maxVal = cvGetReal2D(imgBuffer, objCenter.y, objCenter.x);
            //if (abs(peak.x-objCenter.x)>convWidth/2 || abs(peak.y-objCenter.y)>convHeight/2) continue;

            //mask out peak neighbor area
            cvAnd(imgWeight, imgObjMask, imgBuffer, NULL);
            //cvSaveImage("c:\\CTracker\\imgBuffer_And.bmp", imgBuffer);

            cvSmooth(imgBuffer, imgBuffer, CV_GAUSSIAN, convWidth, convHeight,0);
            //cvSaveImage("c:\\CTracker\\imgBuffer_conv2.bmp", imgBuffer);

            //get second peak location;
            cvMinMaxLoc(imgBuffer, NULL, &maxVal2, NULL, &secondPeak, imgObjMask);

            peakDiff = (int)(maxVal - maxVal2);
            if (peakDiff > maxPeakDiff){
                maxPeakDiff = peakDiff;
                maxPeakDiffIdx = featIdx;
                //get weight image for display purpose
                cvCopy(imgWeight, m_pWeightImg,NULL);
                //cvSaveImage("c:\\CTracker\\gWeightImg_out.bmp", gWeightImg);
            }
        }//end for loop

        //record results into global variables for tracking purpose
        m_MaxFeatIdx[0] = maxPeakDiffIdx;

        //record the logratio of max score feature
        for(j=0;j<BINNUM;j++){//for each histogram bin
            m_MaxFeatRatio[0][j] = logRatio[maxPeakDiffIdx][j];
        }

        //release memory
        cvReleaseImage(&imgWeight);
        cvReleaseImage(&imgBuffer);

        //reset Roi
        cvResetImageROI(imgInput);
        cvResetImageROI(imgObjMask);
        cvResetImageROI(m_pRatioImg);
        cvResetImageROI(m_pWeightImg);
    }//end block of PeakDiff
}

void CTracker::TrackNextFrame(IplImage* inImg, RECT inStartBox, TrkResult *outResult)
    //track one frame
{
    int row, col;
    CvScalar pixel;
    double	ratio;
    double  feat;
    int		featbin;
    int		i;
    double	dnewx, dnewy; //new subpixel x,y after meanshift
    int		newx, newy;
    int		featIdx;
    int		ratioIdx;
    double  maxscore;
    int		xstart, ystart;
    int		halfwid, halfhgt;
    double	candx[3], candy[3];
    int		numOfFeat;		//feat number used in tracking
    RECT	bgBox;

    //get outter bg bounding box
    {
        int boxHalfWid, boxHalfHgt;
        int dmax;
        float boxRatio = m_BoxRatio;

        boxHalfWid = (int)fn_Round((inStartBox.right - inStartBox.left)/2);
        boxHalfHgt = (int)fn_Round((inStartBox.bottom - inStartBox.top)/2);
        dmax = std::max(boxHalfWid, boxHalfHgt);

        bgBox.left = inStartBox.left - (int)fn_Round(dmax*boxRatio);
        bgBox.right = inStartBox.right + (int)fn_Round(dmax*boxRatio);
        bgBox.top = inStartBox.top - (int)fn_Round(dmax*boxRatio);
        bgBox.bottom = inStartBox.bottom + (int)fn_Round(dmax*boxRatio);
        utl_RectCheckBound(&bgBox,inImg->width, inImg->height);
    }


    //loop over max PeakDiff featus
    numOfFeat=1;
    cvSetZero(m_pRatioImg);
    cvSetZero(m_pWeightImg);
    for(i=0;i<numOfFeat;i++){
        //		i = 1; //second max
        featIdx  = m_MaxFeatIdx[i];		//max feature index in featlist;
        maxscore = m_MaxFeatScore[i];	//max feature ratio score
        ratioIdx = i;					//log ratio index;

        //get ratio image
        for(col=bgBox.left;col<bgBox.right;col++){
            for(row=bgBox.top;row<bgBox.bottom;row++){
                pixel = cvGet2D(inImg, row, col); //pixel value is in order of B,G,R
                //calculate feature
                feat = (m_Featlist[featIdx][0]*pixel.val[2]+m_Featlist[featIdx][1]*pixel.val[1]+m_Featlist[featIdx][2]*pixel.val[0]+m_Featlist[featIdx][3])/m_Featlist[featIdx][4];
                featbin = (int)floor(feat/8);
                ratio = m_MaxFeatRatio[ratioIdx][featbin];
                //set ratio image for meanshift purpose
                pixel.val[0] = ratio;
                cvSet2D(m_pRatioImg, row, col, pixel);
            }
        }
        //utl_WriteImage(gRatioImg, "ratioimg.txt");

        //meanshift ratio image
        xstart = (int)fn_Round(((double)inStartBox.left+inStartBox.right)/2);
        ystart = (int)fn_Round(((double)inStartBox.top+inStartBox.bottom)/2);
        halfwid = (int)fn_Round(((double)inStartBox.right - inStartBox.left)/2);
        halfhgt = (int)fn_Round(((double)inStartBox.bottom - inStartBox.top)/2);

        MeanShift(m_pRatioImg, xstart, ystart, halfwid, halfhgt, 1.0, &dnewx, &dnewy);
        candx[i] = dnewx;
        candy[i] = dnewy;

        //get weight image by normalizing ratio image
        if (i==0)//first feature
        {
            double minratio, maxratio;
            double scale, shift;
            CvRect roi;
            roi.x = bgBox.left;
            roi.y = bgBox.top;
            roi.width = bgBox.right-bgBox.left;
            roi.height = bgBox.bottom-bgBox.top;
            cvSetImageROI(m_pRatioImg, roi);
            cvSetImageROI(m_pWeightImg, roi);
            cvMinMaxLoc(m_pRatioImg, &minratio, &maxratio, NULL, NULL, 0);
            shift = 0;
            if (maxratio<=minratio){
                scale = 1;
            }
            else
            {
                scale = 255.0/(fn_Abs(maxratio));
            }
            cvConvertScale(m_pRatioImg, m_pWeightImg, scale, shift);
            cvResetImageROI(m_pRatioImg);
            cvResetImageROI(m_pWeightImg);
        }
    }

    //get median from candidate x,y
    newx = (int)fn_median(candx, numOfFeat);
    newy = (int)fn_median(candy, numOfFeat);

    //return tracking result
    {
        RECT targetBox;
        int  boxWidth, boxHeight;
        int  boxHalfWidth, boxHalfHeight;
        float score;
        float fgRatio;

        //get input box dimension
        boxWidth	= inStartBox.right-inStartBox.left;
        boxHeight	= inStartBox.bottom-inStartBox.top;
        boxHalfWidth	= boxWidth/2;
        boxHalfHeight	= boxHeight/2;

        //get target rect
        targetBox.left		= (long)(newx - boxHalfWidth);
        targetBox.right		= targetBox.left + boxWidth;
        targetBox.top		= (long)(newy - boxHalfHeight);
        targetBox.bottom	= targetBox.top + boxHeight;

        targetBox.left		= std::max(targetBox.left, (long)0);
        targetBox.top		= std::max(targetBox.top,(long)0);
        targetBox.right		= std::max(targetBox.right,(long)1);
        targetBox.bottom	= std::max(targetBox.bottom,(long)1);
        targetBox.right		= std::min(targetBox.right,(long)inImg->width-1);
        targetBox.bottom	= std::min(targetBox.bottom,(long)inImg->height-1);
        targetBox.left		= std::min(targetBox.left,(long)inImg->width-2);
        targetBox.top		= std::min(targetBox.top,(long)inImg->height-2);

        //get occlusion score
        fgRatio = CountFGPixel(m_pWeightImg, targetBox);
        score	= fgRatio/ m_MaskFgRatio;
        score	= std::min((float)1, score);

        //get FG object mask image
        {
            CvRect  roi;
            cvSetZero(m_pImgFgMask);
            cvSetZero(m_pImgObjMask);
            roi.x = targetBox.left;
            roi.y = targetBox.top;
            roi.width = targetBox.right-targetBox.left;
            roi.height = targetBox.bottom-targetBox.top;
            cvSetImageROI(m_pWeightImg, roi);
            cvSetImageROI(m_pImgFgMask, roi);
            cvSetImageROI(inImg, roi);
            cvSetImageROI(m_pImgObjMask, roi);
            cvThreshold(m_pWeightImg, m_pImgFgMask, 0, 255, CV_THRESH_BINARY);
            cvCopy(inImg, m_pImgObjMask, m_pImgFgMask);
            cvResetImageROI(m_pWeightImg);
            cvResetImageROI(m_pImgFgMask);
            cvResetImageROI(inImg);
            cvResetImageROI(m_pImgObjMask);
        }
        outResult->FGMask	= m_pImgFgMask;
        outResult->ObjMask	= m_pImgObjMask;

        //return tracker result
        outResult->targetBox	= targetBox;
        outResult->FGImage		= m_pWeightImg;
        outResult->score		= score;
        outResult->occlusion	= (score<0.6? TRUE:FALSE);
    }
}

void CTracker::InitForFirstFrame(IplImage* imgInput, CvRect CvRectInput)
{
    GetImageWidthHeightXY(imgInput);
    //mean shift track image with half size
    m_pImgCurrentHalf = cvCreateImage(cvSize(m_ImageMaxX/2, m_ImageMaxY/2), 8, 3);
    //mask image
    gImgInitMaskHalf = cvCreateImage(cvSize(m_ImageMaxX/2, m_ImageMaxY/2), 8, 1);
    gImgInitMask = cvCreateImage(cvSize(m_ImageMaxX, m_ImageMaxY), 8, 1);
    //bitmap image
    //gImgBitMap = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);
    utl_ConvertBoxToRect(&m_RectInit, CvRectInput);
    utl_ConvertBoxToRect(&m_RectCurrent, CvRectInput);
    gRectCurrentHalf.left	= m_RectCurrent.left/2;
    gRectCurrentHalf.top	= m_RectCurrent.top/2;
    gRectCurrentHalf.right	= m_RectCurrent.right/2;
    gRectCurrentHalf.bottom	= m_RectCurrent.bottom/2;
    m_pImgCurrent = cvCloneImage(imgInput);
    //get half image for processing
    if (m_pImgCurrent != NULL)
    {
        cvResize(m_pImgCurrent, m_pImgCurrentHalf, CV_INTER_LINEAR);
    }
    // Get Init mask
    GetInitMask();

    TrackInit(m_pImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);

    cvReleaseImage(&m_pImgCurrent);
}

void CTracker::InitForFirstFrame1(IplImage* imgInput, RECT rectInput)
{
    GetImageWidthHeightXY(imgInput);
    //mean shift track image with half size
    m_pImgCurrentHalf = cvCreateImage(cvSize(m_ImageMaxX/2, m_ImageMaxY/2), 8, 3);
    //mask image
    gImgInitMaskHalf = cvCreateImage(cvSize(m_ImageMaxX/2, m_ImageMaxY/2), 8, 1);
    gImgInitMask = cvCreateImage(cvSize(m_ImageMaxX, m_ImageMaxY), 8, 1);


    m_RectInit.top      = rectInput.top;
    m_RectInit.bottom   = rectInput.bottom;
    m_RectInit.left     = rectInput.left;
    m_RectInit.right    = rectInput.right;

    m_RectCurrent.top      = rectInput.top;
    m_RectCurrent.bottom   = rectInput.bottom;
    m_RectCurrent.left     = rectInput.left;
    m_RectCurrent.right    = rectInput.right;

    gRectCurrentHalf.left	= rectInput.left/2;
    gRectCurrentHalf.top	= rectInput.top/2;
    gRectCurrentHalf.right	= rectInput.right/2;
    gRectCurrentHalf.bottom	= rectInput.bottom/2;

    m_pImgCurrent = cvCloneImage(imgInput);

    //get half image for processing
    if (m_pImgCurrent != NULL)
    {
        cvResize(m_pImgCurrent, m_pImgCurrentHalf, CV_INTER_LINEAR);
    }
    // Get Init mask
    GetInitMask();

    TrackInit(m_pImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);

    cvReleaseImage(&m_pImgCurrent);
}
