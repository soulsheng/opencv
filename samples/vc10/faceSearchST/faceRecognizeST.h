
#pragma once

#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>
#include <mcv_faceverify.h>

#include <vector>
#include <string>
#include <map>
using namespace std;

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define FILE_LIST_NAME	"D:\\file\\data\\face\\11faces\\"

class SenseTimeSDK
{
public:

	bool predict( cv::Mat& imageFace, std::vector<int>& lableTop
		, bool bLabel = true, bool bForceGray = true, int n = 5);

	bool faceDetect(cv::Mat& imgIn, cv::Mat& imgOut, std::vector<cv::Mat>& matimg);

	bool train( vector<cv::Mat>& samples, vector<int>& labels, bool bForce = false );

public:
	cv::Mat*	getImage( int nID, bool bLabel = true );
	bool prepareSamples( std::string filelist, bool bPath = false );

	vector<cv::Mat*>& getSamples() { return imageSamples;	}
	vector<int>& getLabels() { return labelSamples;	}

	SenseTimeSDK();
	~SenseTimeSDK();

protected:
	bool checkDataFile();
	bool initialize();
	bool checkTrained();
#if 0
	bool train( std::string filelist );
	bool train( vector<cv::Mat>&	imageSamples );
#endif
	bool release();

	bool save( std::string fileImageFetures );
	bool load( std::string fileImageFetures );


	db_item	getFeature( cv::Mat& imageIn );

private:

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
	vector<cv::Mat*>	imageSamples;
	vector<int>		labelSamples;

	map<int, cv::Mat*>	imageShow;
	map<int, cv::Mat*>	imageItems;

	int db_id ;

	bool bForceGray;
};
