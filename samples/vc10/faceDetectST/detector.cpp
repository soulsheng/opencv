#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>

#include "opencv2/highgui/highgui.hpp"

#include "imagehelper.hpp"

using namespace std;
using namespace sdktest;

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

bool detectST()
{
	Image *img=helper->LoadGrayImage(IMAGE_IN);
	Image *img_color = helper->LoadRGBAImage(IMAGE_IN);
	if(!img || !img_color){
		fprintf(stderr, "fail to read %s\n", IMAGE_IN);
		system( "pause" );
		return -1;
	}

	// detect
	__TIC__();
	mcv_facesdk_multiview_detector(hDetect,img->data,img->cols,img->rows,img->cols,&pface,&countFace);
	__TOC__();

	// draw result
	for ( int i=0;i<countFace;i++){
		helper->DrawRect(img_color,pface[i].Rect.top,pface[i].Rect.left,pface[i].Rect.right,pface[i].Rect.bottom, 3);
		printf("face %d: %d %d %d %d\n", i, pface[i].Rect.top, pface[i].Rect.left, pface[i].Rect.right, pface[i].Rect.bottom);
	}


	helper->SaveImage(IMAGE_OUT,img_color);
	helper->FreeImage(img);
	helper->FreeImage(img_color);
}

int main(int argc, char *argv[])
{
	initialize();

	cv::Mat in = cv::imread( IMAGE_IN );
	cv::imshow( "in", in );
	cv::waitKey( );

	detectST();

	cv::Mat out = cv::imread( IMAGE_OUT );
	cv::imshow( "out", out );
	cv::waitKey( );

	release();

	return 0;
}

