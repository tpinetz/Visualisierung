#include <opencv2/opencv.hpp>
#include <iostream>
#include "Common.hpp"
#include "Detection.hpp"
#include "FaceGeometry.hpp"


int main(int argc, char** argv)
{

	try
	{
		// read front and side image
		cv::Mat front = cv::imread("input/haraldFront.jpg");
		cv::Mat side = cv::imread("input/haraldSide.jpg");

		// detect face geometry
		Face3D::Detection detection(front, side);
		Face3D::FaceGeometry faceGeometry = detection.detectFaceGeometry();

		// save as file so that the second program can load the geometry to adjust the generic 3d model
		faceGeometry.toFile("ipc/faceGeometry.txt");
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

