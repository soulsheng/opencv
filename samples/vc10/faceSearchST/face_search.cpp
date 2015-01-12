
#include "faceRecognizeST.h"

#include <stdio.h>
#include <stdlib.h>

#define IMAGE_FACE_TEST		"f:\\faces\\s1\\18.bmp"
//#define IMAGE_FACE_TEST		"f:\\faces\\s2\\0018.jpg"
#define LABLE_COUNT			5

int main(int argc, char const *argv[])
{
	SenseTimeSDK	st;
	if( !st.checkTrained() )
	{
		printf( "failed to train \n" );
		return false;
	}
	else
		printf( "success to train \n" );


	/* query */
	vector<int>	idResult;
	cv::Mat faceTest = cv::imread( IMAGE_FACE_TEST );
	cv::imshow( "in", faceTest );
	cv::waitKey();

	bool bLabel = false;
	st.predict( faceTest, idResult, bLabel, LABLE_COUNT );
	
	cout << "predict " << IMAGE_FACE_TEST << endl;

	for ( int i = 0; i< idResult.size(); i++ )
	{
		printf( "id = %d, ", idResult[i] );
	}

	cv::imshow( "out", *st.getImage(idResult[0], bLabel) );
	cv::waitKey();

	st.release();

	system( "pause" );
	return 0;
}
