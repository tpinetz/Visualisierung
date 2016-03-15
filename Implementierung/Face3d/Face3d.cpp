#include "Detection.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "Common.hpp"



int main(int argc, char** argv)
{

	cv::Mat front = cv::imread("input/front.jpg");
	

	Face3D::Detection detection(front, front);
	detection.doAllSteps();



	return 0;
}

