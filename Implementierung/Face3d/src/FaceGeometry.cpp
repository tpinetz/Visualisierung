#include "FaceGeometry.hpp"
#include <fstream>

namespace Face3D
{

	//-------------------------------------------------------------------------
	void FaceGeometry::merge3d()
	{	/*
		// very rough approximation of 3d points - just to get the  prototype running

		// eyes
		leftEye = cv::Point3d(frontLeftEye.x, frontLeftEye.y, sideEye.x);
		rightEye = cv::Point3d(frontRightEye.x, frontRightEye.y, sideEye.x);

		// nose
		nose = cv::Point3d((frontRightEye.x + frontLeftEye.x)/2,sideNoseTip.y,sideNoseTip.x); // TODO: alignment needed between front and side image!

		// mouth
		mouth = cv::Point3d(frontMouth.x, frontMouth.y, (sideEye.x+sideNoseTip.x)/2); // another very rough approximation!
		*/
	}


	//-------------------------------------------------------------------------
	void FaceGeometry::toFile(const std::string& fn)
	{
		std::ofstream f(fn.c_str());

		//leftEye, rightEye, nose, mouth
		pointToFile(f, leftEye);
		pointToFile(f, rightEye);
		pointToFile(f, nose);
		pointToFile(f, mouth);
	}


	//-------------------------------------------------------------------------
	void FaceGeometry::fromFile(const std::string& fn)
	{
		std::ifstream f(fn.c_str());

		leftEye=fileToPoint(f);
		rightEye = fileToPoint(f);
		nose = fileToPoint(f);
		mouth = fileToPoint(f);
	}


	//-------------------------------------------------------------------------
	cv::Point3d fileToPoint(std::ifstream& f)
	{
		cv::Point3d p;

		f >> p.x;
		f >> p.y;
		f >> p.z;

		return p;
	}


	//-------------------------------------------------------------------------
	void pointToFile(std::ofstream& f, const cv::Point3d& p)
	{
		f << p.x << "\n";
		f << p.y << "\n";
		f << p.z << "\n";
	}



	cv::Point2d FaceGeometry::getDetectedPoint(DetectedPoints detectedPoint) const
	{
		return cv::Point2d(m_DetectedPoints[detectedPoint].x, m_DetectedPoints[detectedPoint].y); 
	}

	cv::Point FaceGeometry::getDetectedPointInt(DetectedPoints detectedPoint) const
	{
		return cv::Point((cv::Point::value_type)m_DetectedPoints[detectedPoint].x, (cv::Point::value_type)m_DetectedPoints[detectedPoint].y);
	}


	cv::Point3d FaceGeometry::getDetectedPointHomogeneous(DetectedPoints detectedPoint) const
	{
		return m_DetectedPoints[detectedPoint]; 
	}

	void FaceGeometry::setDetectedPoint(DetectedPoints detectedPoint, const cv::Point2d& p)
	{ 
		m_DetectedPoints[detectedPoint].x = p.x; 
		m_DetectedPoints[detectedPoint].y = p.y; 
		m_DetectedPoints[detectedPoint].z = 1.0; 
	}

	void FaceGeometry::setDetectedPoint(DetectedPoints detectedPoint, const cv::Point3d& p)
	{ 
		m_DetectedPoints[detectedPoint].x = p.x; 
		m_DetectedPoints[detectedPoint].y = p.y; 
		m_DetectedPoints[detectedPoint].z = 1.0; 
	}

	void FaceGeometry::transform(DetectedPoints point, const cv::Mat& transform)
	{
		
		cv::Mat transform3x3(transform);
		if (transform3x3.rows == 2)
		{
			cv::Mat newrow=cv::Mat::zeros(1, 3, CV_64F);
			newrow.at<double>(0,2) = 1;
			transform3x3.push_back(newrow);
		}
		


		cv::Mat p(m_DetectedPoints[point],false);
		cv::Mat res = transform3x3*p;
		res.copyTo(cv::Mat(m_DetectedPoints[point],false));
	}

}