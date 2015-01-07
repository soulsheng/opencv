#include "faceDetectST.h"
#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "imagehelper.hpp"

using namespace std;
using namespace sdktest;
using namespace cv;

/// \lib facesdk
/// \api mcv_facesdk_multiview_detector
/// \desc 能够检测出不同姿态方位的人脸在图中的位置 

#define IMAGE_IN	"../data/bighero6.jpg"
#define IMAGE_OUT	"../data/out.jpg"

ImageHelper *helper;
mcv_handle_t hDetect;
PMCV_FACERECT pface=NULL;
unsigned int countFace=0;

bool initialize()
{
	helper = new ImageHelperBackend();

	hDetect = mcv_facesdk_create_multiview_detector_instance_from_resource(true, 2);

	return true;
}

bool release()
{
	delete helper;

	// release the memory of result
	mcv_facesdk_release_multiview_result(pface,countFace);

	// destroy handle
	mcv_facesdk_destroy_multiview_instance(hDetect);

	return true;
}

bool faceDetectST(cv::Mat& imgIn, cv::Mat& imgOut, vector<cv::Mat>& matimg)
{
	cv::Mat gray;
	cvtColor( imgIn, gray, CV_BGR2GRAY );
	cv::Mat *img = &gray;

	// detect
	__TIC__();
	mcv_facesdk_multiview_detector(hDetect,img->data,img->cols,img->rows,img->cols,&pface,&countFace);
	__TOC__();

	// draw result
	for ( int i=0;i<countFace;i++){
	rectangle( imgOut, 
		cvPoint( pface[i].Rect.left, pface[i].Rect.top ),
		cvPoint( pface[i].Rect.right, pface[i].Rect.bottom ) ,
		CV_RGB(0,0,255), 3, 8, 0);

	cv::Mat imgROI( imgIn, Rect(
		pface[i].Rect.left, pface[i].Rect.top ,
		pface[i].Rect.right - pface[i].Rect.left, pface[i].Rect.bottom - pface[i].Rect.top ));
	//printf("width = %d \n", pface[i].Rect.right - pface[i].Rect.left);
	matimg.push_back(imgROI);

	}

	return true;
}
