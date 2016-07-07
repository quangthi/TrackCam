#ifndef UTILITY_H
#define UTILITY_H
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm> 
#include "math.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include <DIRECT.H>

//general auxillary fuction
double  fn_Round(double input);
double	fn_kthSmallest(double a[], int n, int k);
double	fn_median(double a[], int n);
double	fn_Abs(double input);
char*	fn_DoubleToStr(double input, int precision);
void	fn_DoubleToStr2(double input, int precision, char* strOutput);

//RECT operation
//scale a rect by a zoom factor
void utl_RectZoom(RECT *ioRect, float inZoomFactor, int imgWidth, int imgHeight);
//check the boundary limit of RECT
void utl_RectCheckBound(RECT *ioRect, int imgWidth, int imgHeight);
//rect at the same center with different size
void utl_RectSizeAdjust(RECT *ioRect, int dx, int dy, int imgWidth, int imgHeight);
//rect increase size of border
void utl_RectSizeIncrease(RECT *ioRect, int border, int imgWidth, int imgHeight);

//check if pixel is valid in bound of image
void utl_PixelCheckBound(int *pixelX, int *pixelY, int imgWidth, int imgHeight);
void utl_PixelCheckBound_Float(float *pixelX, float *pixelY, int imgWidth, int imgHeight);

void utl_IndexNewDir(char *ioNewDirPath);

int utl_Min3(int a, int b, int c);
int utl_Max3(int a, int b, int c);

//write single channel image value to a txt file
void utl_WriteImage(IplImage *imgInput, char* filename);
void utl_DrawLine(IplImage *imgInput, POINT pt1, POINT pt2, CvScalar pixval);

void utl_ConvertRectToBox(RECT RectInput, CvRect *CvRectInput);
void utl_ConvertBoxToRect(RECT *RectInput, CvRect CvRectInput);


// For tracking
typedef struct TrkResult
{
	RECT		targetBox;
	IplImage	*FGImage;
	IplImage	*FGMask;
	IplImage	*ObjMask;
	float		score;
	BOOL		occlusion; //flag for occusion
	int			x0; //obj center x
	int			y0; //obj center y
	int			axisA; //ellipse axis A
	int			axisB; //ellipse axis B
	double		angle; //ellipse angle
} TrkResult;
#endif
