#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>

#include "opencv2/highgui/highgui.hpp"

#include "faceDetectST.h"

/// \lib facesdk
/// \api mcv_facesdk_multiview_detector
/// \desc 能够检测出不同姿态方位的人脸在图中的位置 

#define IMAGE_IN	"../data/bighero6.jpg"
#define IMAGE_OUT	"../data/out.jpg"


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

