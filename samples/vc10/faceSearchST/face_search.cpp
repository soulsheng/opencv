
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


	/* query */
	vector<int>	idResult;
	cv::Mat faceTest = cv::imread( IMAGE_FACE_TEST );
	cv::imshow( "in", faceTest );
	cv::waitKey();

	bool bLabel = true;
	st.predict( faceTest, idResult, bLabel, true, LABLE_COUNT );
	
	cout << "predict " << IMAGE_FACE_TEST << endl;

	for ( int i = 0; i< idResult.size(); i++ )
	{
		printf( "id = %d, ", idResult[i] );
	}

	cv::Mat*pImg  = st.getImage(idResult[0], bLabel);
	if( pImg )
	{
		cv::imshow( "out", *pImg );
		cv::waitKey();
	}

	st.release();

	system( "pause" );
	return 0;
}
