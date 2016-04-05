#include "Detection.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "Common.hpp"



int main(int argc, char** argv)
{

	try
	{
		cv::Mat front = cv::imread("input/haraldFront.jpg");
		cv::Mat side = cv::imread("input/haraldSide.jpg");


		Face3D::Detection detection(front, side);
		detection.doAllSteps();
	}
	catch (std::exception e)
	{
		std::cout<<"Exception: "<<e.what()<<"\n";
	}
	catch (...)
	{
		std::cout << "Exception\n";
	}
	



	return 0;
}

