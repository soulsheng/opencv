
#include "faceRecognizeST.h"

#include <stdio.h>
#include <stdlib.h>

#define IMAGE_FACE_TEST		"f:\\faces\\s1\\15.bmp"
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
	if ( !st.predict() )
		return false;

	vector<int>	labelResult;
	cv::Mat faceTest = cv::imread( IMAGE_FACE_TEST );

	st.predict( faceTest, labelResult, LABLE_COUNT );

	for ( int i = 0; i< labelResult.size(); i++ )
	{
		printf( "id = %d, ", labelResult[i] );
	}

	st.release();

	system( "pause" );
	return 0;
}
