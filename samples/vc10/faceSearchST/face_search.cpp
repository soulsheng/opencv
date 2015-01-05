
#include "faceRecognizeST.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{

	if( !checkTrained() )
	{
		printf( "failed to train \n" );
		return false;
	}
	else
		printf( "success to train \n" );


	/* query */
	if ( !predict() )
		return false;

	predict();
	predict();

	release();

	system( "pause" );
	return 0;
}
