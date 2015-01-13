
#include "faceRecognizeST.h"

#include <stdio.h>
#include <stdlib.h>

//#define IMAGE_FACE_TEST		"f:\\faces\\s1\\01.bmp"
#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s1\\03.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s2-gray\\0020.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s3\\07.bmp"

//#define IMAGE_FACE_TEST		"f:\\faces\\s2\\0018.jpg"
#define LABLE_COUNT			5

int main(int argc, char const *argv[])
{
	SenseTimeSDK	st;

	st.prepareSamples( FILE_LIST_NAME, true );
	vector<cv::Mat> samples = st.getSamples();
	vector<int> labels = st.getLabels();
	cout << "samples.size() = " << samples.size() << endl;
	cout << "labels.size() = " << labels.size() << endl;

	st.train( samples, labels, true ); // cost 1.4s for each face image(144*144) to get feature(training)

	/* query */
	vector<int>	idResult;
	cv::Mat faceTest = cv::imread( IMAGE_FACE_TEST );
	cv::imshow( "in", faceTest );
	cv::waitKey();

	bool bLabel = true;
	st.predict( faceTest, idResult, bLabel );
	
	cout << "predict " << IMAGE_FACE_TEST << endl;

	for ( int i = 0; i< idResult.size(); i++ )
	{
		printf( "id = %d, ", idResult[i] );
	}
	cv::Mat*pImg = NULL;
	if( idResult.size() )
	{
		pImg  = st.getImage(idResult[0], bLabel);
		if( pImg )
		{
			cv::imshow( "out", *pImg );
			cv::waitKey();
		}
	}

	system( "pause" );
	return 0;
}
