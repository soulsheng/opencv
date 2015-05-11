
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

#include "singleton.h"

#define FILE_LIST_NAME	"D:\\file\\data\\face\\11faces\\"
#include "helper_timer.h"

class SenseTimeSDK : public Singleton<SenseTimeSDK>
{
public:
	static SenseTimeSDK& getSingleton(void);
	static SenseTimeSDK* getSingletonPtr(void);

	bool predict( cv::Mat& imageFace, std::vector<int>& lableTop
		, bool bLabel = true, bool bForceGray = true, int n = 5);

	bool faceDetect(cv::Mat& imgIn, cv::Mat& imgOut, std::vector<cv::Mat>& matimg, int nInstance);

	bool train( vector<cv::Mat>& samples, vector<int>& labels, bool bForce = false );

	bool trainAdd( vector<cv::Mat>& samples, vector<int>& labels );

	void setScoreLine( int score );
	void setRatioThreshold( float ratio );
	void setAngleMax( float angle );

public:
	cv::Mat*	getImage( int nID, bool bLabel = true );
	bool prepareSamples( std::string filelist, vector<cv::Mat>& samples, vector<int>& labels,
		int nIDMember, int nCountEach=10);

	bool prepareSamples( std::string fileList, vector<cv::Mat>& samples, vector<int>& labels );

	int findLabelByItemIdx( int idx );

	SenseTimeSDK(int nInstanceCount=2);
	~SenseTimeSDK();

protected:
	bool checkDataFile();
	bool initialize();
	bool checkTrained();

	bool release();

	bool save( std::string fileImageFetures );
	bool load( std::string fileImageFetures );


	db_item	getFeature( cv::Mat& imageIn );
	void scale( mcv_rect_t& rect, float s, cv::Mat& imgIn );

	bool testFrontal( cv::Mat& mat, mcv_rect_t& rect );

private:
	mcv_handle_t hAlignmentor;

	mcv_handle_t vinst;
	vector<db_item> items;
	
	mcv_handle_t hIndex;
	bool bTrain;
	bool bInitialized;
	bool bReleased;


	// train
	vector<cv::Mat*>	imageSamples;
	vector<int>		labelSamples;

	map<int, cv::Mat*>	imageShow;
	

	int db_id ;

	bool bForceGray;

	StopWatchInterface*				timerCurrent;
	vector<StopWatchInterface *>	timer;
	typedef vector<StopWatchInterface *>::iterator TimeItr;

	vector<mcv_handle_t>		m_nInstance;
	int	nScoreLine;			// minimum score of match
	float fRatioThreshold;	// minimum ratio of face area and full image area, unit %
	float s;				// scale time

	int npoint;
	mcv_pointf_t* points;
	float angleThreshold;	// angle max as frontal
};
