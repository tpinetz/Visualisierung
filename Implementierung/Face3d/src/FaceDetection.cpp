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
		Face3D::Detection::DetectFaceResult detectFaceResult = detection.detectFace();

		// save as file so that the second program can load the geometry to adjust the generic 3d model
		detectFaceResult.faceGeometry.toFile("ipc/faceGeometry.txt");
		cv::imwrite("ipc/front.jpg",detectFaceResult.textureFront);
		cv::imwrite("ipc/side.jpg", detectFaceResult.textureSide);

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

