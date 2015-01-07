
#pragma	once

#include "opencv2/highgui/highgui.hpp"
#include <vector>

bool initialize();

bool faceDetectST(cv::Mat& imgIn, cv::Mat& imgOut, std::vector<cv::Mat>& matimg);

bool release();