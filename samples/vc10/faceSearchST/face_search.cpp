
#include "faceRecognizeST.h"

#include <stdio.h>
#include <stdlib.h>

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

	st.predict();
	st.predict();

	st.release();

	system( "pause" );
	return 0;
}
