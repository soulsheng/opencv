
#pragma once

#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>
#include <mcv_faceverify.h>

#include <vector>
#include <string>
using namespace std;

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


class SenseTimeSDK
{
public:
	bool checkTrained();

	bool train( std::string filelist );
	bool train( vector<cv::Mat>&	imageSamples );

	bool predict();
	bool predict( cv::Mat& imageFace, std::vector<int>& lableTop, int n = 5);

	bool faceDetect(cv::Mat& imgIn, cv::Mat& imgOut, std::vector<cv::Mat>& matimg);

	bool release();

	SenseTimeSDK();
	~SenseTimeSDK();

protected:
	bool checkDataFile();
	bool initialize();

	bool save( std::string fileImageFetures );
	bool load( std::string fileImageFetures );

	bool prepareSamples( std::string filelist );

	db_item	getFeature( cv::Mat& imageIn );

private:

	FILE *flist, *of;
	mcv_handle_t hDetect;
	mcv_handle_t vinst;
	vector<db_item> items;
	vector<string> names;
	mcv_handle_t hIndex;
	bool bTrain;
	bool bInitialized;
	bool bReleased;

	PMCV_FACERECT pface ;
	unsigned int countFace ;

	// train
	vector<cv::Mat>	imageSamples;
	vector<int>		labelSamples;

	int db_id ;

};
