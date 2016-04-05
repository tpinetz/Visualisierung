#pragma once

#include <opencv2/opencv.hpp>

namespace Face3D
{
	class FaceGeometry
	{
	public:
		
		// TODO: put those into methods ... insteaf of just writing/reading the member vars from outside the class!

		// 2d position of components in front image
		cv::Point2d frontLeftEye, frontRightEye, frontMouth;

		// 2d position of components in side image
		cv::Point2d sideEye, sideNoseTip;



		// 3d position: 2d positions must be filled out and merge3d() must be called 

		// 3d position
		cv::Point3d leftEye, rightEye, nose, mouth;


		// combine the points into 3d points
		void merge3d();

		void toFile(const std::string& fn);
		void fromFile(const std::string& fn);
	};

	cv::Point3d fileToPoint(std::ifstream& f);
	void pointToFile(std::ofstream& f, const cv::Point3d& p);
}