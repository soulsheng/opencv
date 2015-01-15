
#include "faceRecognizeST.h"

#include <stdio.h>
#include <stdlib.h>

//#define IMAGE_FACE_TEST		"f:\\faces\\s1\\01.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s1\\03.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s2-gray\\0020.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s3\\07.bmp"

#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\11faces\\18.bmp"
//#define IMAGE_FACE_TEST		"f:\\faces\\s2\\0018.jpg"
#define LABLE_COUNT			5

int main(int argc, char const *argv[])
{
	SenseTimeSDK*	st = NULL;
	if( SenseTimeSDK::getSingletonPtr() == 0 )
	{
		st = new SenseTimeSDK();
	}
	else 
	{
		assert( 0 && "SenseTimeSDK已被创建! " );
	}

#if 0
	vector<cv::Mat> samples;
	vector<int> labels;

	st->prepareSamples( FILE_LIST_NAME, samples, labels, 1, 30 );

	cout << "samples.size() = " << samples.size() << endl;
	cout << "labels.size() = " << labels.size() << endl;

	cv::waitKey();

	st->train( samples, labels, true ); // cost 1.4s for each face image(144*144) to get feature(training)
#elif 1

	for (int i=1;i<=11;i++)
	{
	vector<cv::Mat> samples;
	vector<int> labels;

	st->prepareSamples( FILE_LIST_NAME, samples, labels, i, 30 );

	cout << "samples.size() = " << samples.size() << endl;
	cout << "labels.size() = " << labels.size() << endl;

	cv::waitKey();

	st->trainAdd( samples, labels );
	}

#else
	;
#endif
	/* query */
	vector<int>	idResult;
	cv::Mat faceTest = cv::imread( IMAGE_FACE_TEST );
	cv::imshow( "in", faceTest );
	cv::waitKey();

	bool bLabel = true;
	st->predict( faceTest, idResult, bLabel );
	
	cout << "predict " << IMAGE_FACE_TEST << endl;

	if( idResult.size() )
		cout << "found" << endl;
	else
		cout << "not found" << endl;

	for ( int i = 0; i< idResult.size(); i++ )
	{
		printf( "id = %d, ", idResult[i] );
	}
#if 0
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
#endif
	system( "pause" );

	if ( st != NULL )
	{
		delete st;
	}

	return 0;
}
