#pragma once

#include <opencv2/opencv.hpp>

namespace Face3D
{
	class FaceGeometry
	{
	public:
		// the points we are detecting so far ...
		enum DetectedPoints{FrontLeftEye,FrontRightEye,FrontMouth,SideEye,SideNoseTip,InvalidPoint};

		// set and get a detected point
		cv::Point2d getDetectedPoint(DetectedPoints detectedPoint) const;
		cv::Point getDetectedPointInt(DetectedPoints detectedPoint) const;
		cv::Point3d getDetectedPointHomogeneous(DetectedPoints detectedPoint) const;
		void setDetectedPoint(DetectedPoints detectedPoint, const cv::Point2d& p);
		void setDetectedPoint(DetectedPoints detectedPoint, const cv::Point3d& p);

		// apply an affine transform to a point
		void transform(DetectedPoints point, const cv::Mat& transform);


		// combine the points into 3d points
		void merge3d();

		void toFile(const std::string& fn);
		void fromFile(const std::string& fn);


		// TODO
		cv::Rect sideSkinRegion;
		cv::Rect frontSkinRegion;

	private:
		// the 2d points in homogeneous coordinates such that an affine transformation can be applied
		cv::Point3d m_DetectedPoints[InvalidPoint];		

		// 3d position
		cv::Point3d leftEye, rightEye, nose, mouth;


		
	};

	cv::Point3d fileToPoint(std::ifstream& f);
	void pointToFile(std::ofstream& f, const cv::Point3d& p);
}