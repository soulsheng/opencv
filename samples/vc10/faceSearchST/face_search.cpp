
#include "faceRecognizeST.h"

#include <stdio.h>
#include <stdlib.h>

//#define IMAGE_FACE_TEST		"f:\\faces\\s1\\01.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s1\\03.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s2-gray\\0020.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\5person\\s3\\07.bmp"

//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\11faces\\168.bmp"
#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\20150507\\20150507_092216_1.bmp"
//#define IMAGE_FACE_TEST		"D:\\file\\data\\face\\sense_2\\sensetime_temp_2.bmp"

#define LABLE_COUNT			5
#define FILE_LIST_NAME_2K	"D:\\file\\data\\face\\CardSample\\1.txt"
#define FILE_LIST_NAME_1	"D:\\file\\data\\face\\sense\\1.txt"
#define FILE_LIST_NAME_N	"D:\\file\\data\\face\\20150507\\1.txt"

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
#elif 0

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

#elif 0
	vector<cv::Mat> samples;
	vector<int> labels;
	cv::Mat	img = cv::imread("D:\\file\\data\\face\\sense_2\\sense_2.bmp");
	samples.push_back( img );
	labels.push_back( 13 );
	st->trainAdd( samples, labels );
#elif 1

	vector<cv::Mat> samples;
	vector<int> labels;

	st->prepareSamples( FILE_LIST_NAME_N, samples, labels );

	//st->train( samples, labels, true ); // cost 1.4s for each face image(144*144) to get feature(training)

#endif
	/* query */
	for( int i = 0; i< samples.size(); i++ ) {
	vector<int>	idResult;
	//cv::Mat faceTest = cv::imread( IMAGE_FACE_TEST );
	//cv::imshow( "in", faceTest );
	//cv::waitKey();

	st->setScoreLine( -20 );
	bool bLabel = true;
	st->predict( samples[i], idResult, bLabel );
	
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

	}

	system( "pause" );

	if ( st != NULL )
	{
		delete st;
	}

	return 0;
}
