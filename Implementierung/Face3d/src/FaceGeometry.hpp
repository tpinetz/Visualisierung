#pragma once

#include <opencv2/opencv.hpp>

namespace Face3D
{
	/** class which calculates the 2d points of the facial components and merges them into 3d coordinates */
	class FaceGeometry
	{
	public:
		/** the facial components in the front and side image */
		enum DetectedPoints{ FrontLeftEye, FrontRightEye, FrontMouth, FrontLeftCheek, FrontRightCheek, SideEye, SideNoseTip, SideChin, SideBack, TextureLeftEye, TextureRightEye, TextureChin, InvalidPoint};

		/** get 2d point with datatype double */
		cv::Point2d getDetectedPoint(DetectedPoints detectedPoint) const;

		/** get 2d point with datatype int */
		cv::Point getDetectedPointInt(DetectedPoints detectedPoint) const;

		/** get 3d point (homogeneous coordinates) with datatype double */
		cv::Point3d getDetectedPointHomogeneous(DetectedPoints detectedPoint) const;

		/** set 2d point with datatype double */
		void setDetectedPoint(DetectedPoints detectedPoint, const cv::Point2d& p);

		/** set 3d (homogeneous coordinates) point with datatype double */
		void setDetectedPoint(DetectedPoints detectedPoint, const cv::Point3d& p);

		/** apply an affine transform to a point. can be a 2x3 or 3x3 matrix. */
		void transform(DetectedPoints point, const cv::Mat& transform);

		/** combine the points into 3d points */
		void merge3d();

		/** ipc - serialize current object state to file */
		void toFile(const std::string& fn);

		

	private:
		cv::Point3d m_DetectedPoints[InvalidPoint];	 ///< the 2d points in homogeneous coordinates such that an affine transformation can be applied
		cv::Point3d leftEye, rightEye, nose, mouth, chin, faceDimensions; ///< 3d position
		cv::Rect sideSkinRegion; ///< region of the skin in the side image
		cv::Rect frontSkinRegion; ///< region of the skin in the front image


		/** save a single point to a file */
		void pointToFile(std::ofstream& f, const cv::Point3d& p);
	};

	
}