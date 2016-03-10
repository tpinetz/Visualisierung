#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv)
{

	std::cout << "Hallo\n";

	// teste OpenCV
	cv::Mat img = cv::imread("input/HelloWorld.bmp");
	cv::imshow("Vis2",img);
	cv::waitKey(0);

	return 0;
}

