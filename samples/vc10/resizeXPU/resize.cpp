
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;


void main()
{

	string inputName = "../../images/151.bmp";

	Mat image, imageResized;
	image = imread(inputName);

	GpuMat image_gpu, image_gpuResized;


	float	scale = 0.5f;
	bool useGPU = false;

	Size sz( cvRound(image.cols * scale), cvRound(image.rows * scale) );
	
	double t = (double)cvGetTickCount();
	if( !useGPU )
	{
		resize( image, imageResized, sz );
	}
	else
	{
		image_gpu.upload( image );
		resize( image_gpu, image_gpuResized, sz );
		image_gpuResized.download( imageResized );
	}
	t = (double)cvGetTickCount() - t;
	printf( "resize time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );


	imshow("result", imageResized );

	waitKey();
}