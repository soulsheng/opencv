#include <iostream>
#include <cassert>
#include <vector>

#include "opencv2/highgui/highgui.hpp"

#include "../faceSearchST/faceRecognizeST.h"

/// \lib facesdk
/// \api mcv_facesdk_multiview_detector
/// \desc 能够检测出不同姿态方位的人脸在图中的位置 

#define IMAGE_IN	"../data/bighero6.jpg"
#define IMAGE_OUT	"../data/out.jpg"


int main(int argc, char *argv[])
{
	SenseTimeSDK st;
	st.setRatioThreshold( 0.25 );

	cv::Mat in = cv::imread( IMAGE_IN );
	cv::imshow( "in", in );
	cv::waitKey( );

	cv::Mat out(in);
	std::vector<cv::Mat> faces;
	st.faceDetect( in, out, faces );

	cv::imshow( "out", out );
	cv::waitKey( );

	for (int i=0; i<faces.size(); i++ )
	{
		char title[20];
		sprintf( title, "face%d.jpg",i);
		cv::imshow( title, faces[i] );
		cv::waitKey( );
		cv::imwrite( title, faces[i] );
	}

	return 0;
}

