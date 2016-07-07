#ifndef CTRACKER_H
#define CTRACKER_H

#include "Utility.h"

#define FEATNUM 49				//overall feature number
#define BINNUM  32				//histogram bin number

class CTracker
{
public:
    //global variable for initialization
    int		m_Featlist[FEATNUM][5];		//store feature combination list
    char	m_Featlabels[FEATNUM][10];	//string label of the feature list
    double  m_PobjFirst[FEATNUM][BINNUM];//Obj histogram on first frame
    double  m_MaxFeatRatio[3][BINNUM];	//selected 3 most ratio features
    double	m_MaxFeatScore[3];			//scores for max ratio features
    int		m_MaxFeatIdx[3];				//index for max ratio features


    //global vars for tracking
    float	m_BoxRatio;			//ratio between forground and background bounding box

    IplImage	*m_pRatioImg;			//logratio image for meanshift purpose
    IplImage	*m_pWeightImg;			//weight image in meanshift process
    IplImage	*m_pImgFgMask;
    IplImage	*m_pImgObjMask;

    float	m_MaskFgRatio;

    int m_ImageMaxX;
    int	m_ImageMaxY;
    int	m_ImageWidth;
    int	m_ImageHeight;


    IplImage	*m_pImgCurrent;
    IplImage	*m_pImgCurrentHalf;

    RECT		m_RectInit;
    RECT		m_RectCurrent;
    RECT		gRectCurrentHalf;

    IplImage	*gImgInitMaskHalf;
    IplImage	*gImgInitMask;
    TrkResult	m_TrkResult;

    //poly region handle
    HRGN		m_RegionInit;
    HRGN		m_RegionCurrent;
    HRGN		m_RegionCurrentHalf;
public:
    CTracker();
    void TrackNextFrame(IplImage* inImg, RECT inStartBox, TrkResult *outResult);
    void InitForFirstFrame(IplImage* imgInput, CvRect CvRectInput);
    void InitForFirstFrame1(IplImage* imgInput, RECT rectInput);
private:
    void GenColorFeatures();
    void GetImageWidthHeightXY(IplImage* imgInput);
    void MeanShift(IplImage *imgInput, int xstart, int ystart, int hx, int hy, double eps, double *modex, double *modey);
    float CountFGPixel(IplImage *inImg, RECT inRect);
    void CreateRegionMask(RECT inRect, IplImage *outImgMask);
    void GetInitMask();
    void TrackInit(IplImage* imgInput, IplImage* imgObjMask, RECT inTargetBox);
};

#endif // CTRACKER_H
