#include <opencv2/opencv.hpp>
#include <iostream>
#include "Common.hpp"
#include "Detection.hpp"
#include "FaceGeometry.hpp"
#include <Windows.h>

/** show error message to user */
void showErrorMsg(const std::string& txt)
{
	std::cout << "Error: " << txt << "\n";

	// on win32: show message box
#ifdef _WIN32
	MessageBoxA(0, txt.c_str(), "Face3d: error message", MB_OK);
#endif
}

/** main function, reading from input directory, writing to ipc directory */
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
		showErrorMsg(e.what());
	}
	catch (...)
	{
		showErrorMsg("unknown error");		
	}
	

	return 0;
}

