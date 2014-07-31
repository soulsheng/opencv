/**
 * file Smoothing.cpp
 * brief Sample code for simple filters
 * author OpenCV team
 */
#include <iostream>
#include <vector>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"

using namespace std;
using namespace cv;

#include "timerC.h"
#define		ENABLE_TIME_STEP	1 	// 测量某个范围的总时间

/// Global Variables
int DELAY_CAPTION = 1500;
int DELAY_BLUR = 100;
int MAX_KERNEL_LENGTH = 31;

Mat src; Mat dst;
char window_name[] = "Smoothing Demo";

/// Function headers
int display_caption( const char* caption );
int display_dst( int delay );


/**
 * function main
 */
int main( void )
{
  namedWindow( window_name, CV_WINDOW_AUTOSIZE );

  /// Load the source image
  src = imread( "../images/lena.jpg", 1 );	//	define in loadsave.cpp

  if( display_caption( "Original Image" ) != 0 ) { return 0; }

  dst = src.clone();
  if( display_dst( DELAY_CAPTION ) != 0 ) { return 0; }

#if 0
  /// Applying Homogeneous blur
  if( display_caption( "Homogeneous Blur" ) != 0 ) { return 0; }

  for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
      { blur( src, dst, Size( i, i ), Point(-1,-1) );
        if( display_dst( DELAY_BLUR ) != 0 ) { return 0; } }


  /// Applying Gaussian blur
  if( display_caption( "Gaussian Blur" ) != 0 ) { return 0; }

  for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
      { GaussianBlur( src, dst, Size( i, i ), 0, 0 );
        if( display_dst( DELAY_BLUR ) != 0 ) { return 0; } }
#endif


  /// Applying Median blur
  if( display_caption( "Median Blur" ) != 0 ) { return 0; }

  for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
  { 

		timerC  timeTest;
#if ENABLE_TIME_STEP
		timeTest.start();
#endif
		printf(" \n\nfilter window size = %d \n",3);
		medianBlur ( src, dst, 3 );//	1.8 ms (450,340,3); 2.8 ms (520,540,3)

#if ENABLE_TIME_STEP	// xx ms
		timeTest.stop();
		printf( "\n Time of medianBlur is : %.3f\n", timeTest.getTime() );
#endif	

        if( display_dst( DELAY_BLUR ) != 0 ) { return 0; } 
  }


#if 0
  /// Applying Bilateral Filter
  if( display_caption( "Bilateral Blur" ) != 0 ) { return 0; }

  for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
      { bilateralFilter ( src, dst, i, i*2, i/2 );
        if( display_dst( DELAY_BLUR ) != 0 ) { return 0; } }
#endif

  /// Wait until user press a key
  display_caption( "End: Press a key!" );

  waitKey(0);

  return 0;
}

/**
 * @function display_caption
 */
int display_caption( const char* caption )
{
  dst = Mat::zeros( src.size(), src.type() );
  putText( dst, caption,
           Point( src.cols/4, src.rows/2),
           CV_FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255) );

  imshow( window_name, dst );
  int c = waitKey( DELAY_CAPTION );
  if( c >= 0 ) { return -1; }
  return 0;
}

/**
 * @function display_dst
 */
int display_dst( int delay )
{
  imshow( window_name, dst );
  int c = waitKey ( delay );
  if( c >= 0 ) { return -1; }
  return 0;
}
