
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

	bool predict();

	bool faceDetect(cv::Mat& imgIn, cv::Mat& imgOut, std::vector<cv::Mat>& matimg);

	bool release();

	SenseTimeSDK();
	~SenseTimeSDK();

protected:
	bool checkDataFile();
	bool initialize();
	bool train();
	bool save( std::string fileItems, std::string fileNames );
	bool load( std::string fileItems, std::string fileNames );

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

};
