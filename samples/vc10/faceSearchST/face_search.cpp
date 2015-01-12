
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
	vector<int>	labelResult;
	cv::Mat faceTest = cv::imread( IMAGE_FACE_TEST );
	cv::imshow( "in", faceTest );
	cv::waitKey(5);

	st.predict( faceTest, labelResult, LABLE_COUNT );
	
	cout << "predict " << IMAGE_FACE_TEST << endl;

	for ( int i = 0; i< labelResult.size(); i++ )
	{
		printf( "id = %d, ", labelResult[i] );
	}

	cv::imshow( "out", *st.getImage(labelResult[0]) );
	cv::waitKey();

	st.release();

	system( "pause" );
	return 0;
}
