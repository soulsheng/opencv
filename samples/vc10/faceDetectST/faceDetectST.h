
#pragma	once

#include "opencv2/highgui/highgui.hpp"

bool initialize();

bool faceDetectST(cv::Mat& imgIn, cv::Mat& imgOut);

bool release();