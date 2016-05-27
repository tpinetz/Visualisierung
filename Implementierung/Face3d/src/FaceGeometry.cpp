#include "FaceGeometry.hpp"
#include <fstream>

namespace Face3D
{

	//-------------------------------------------------------------------------
	void FaceGeometry::merge3d()
	{	
		// very rough approximation of 3d points - just to get the  prototype running

		// eyes
		leftEye = cv::Point3d(m_DetectedPoints[FrontLeftEye].x, m_DetectedPoints[FrontLeftEye].y, m_DetectedPoints[SideEye].x);
		rightEye = cv::Point3d(m_DetectedPoints[FrontRightEye].x, m_DetectedPoints[FrontRightEye].y, m_DetectedPoints[SideEye].x);

		// nose
		nose = cv::Point3d((m_DetectedPoints[FrontRightEye].x + m_DetectedPoints[FrontLeftEye].x) / 2.0, m_DetectedPoints[SideNoseTip].y, m_DetectedPoints[SideNoseTip].x); // TODO: alignment needed between front and side image!
	
		// mouth
		mouth = cv::Point3d(m_DetectedPoints[FrontMouth].x, m_DetectedPoints[FrontMouth].y, (m_DetectedPoints[SideEye].x + m_DetectedPoints[SideNoseTip].x) / 2); // another very rough approximation!		

		// chin
		chin = cv::Point3d((m_DetectedPoints[FrontRightEye].x + m_DetectedPoints[FrontLeftEye].x) / 2.0, m_DetectedPoints[SideChin].y, m_DetectedPoints[SideChin].x); // TODO: alignment needed between front and side image!

		// face dimension is meassured between: left/right cheek (x), eye/chin (y), chin/backside (z)
		faceDimensions.x = m_DetectedPoints[FrontRightCheek].x - m_DetectedPoints[FrontLeftCheek].x;
		faceDimensions.y = m_DetectedPoints[SideChin].y - m_DetectedPoints[SideEye].y;
		faceDimensions.z = m_DetectedPoints[SideChin].x - m_DetectedPoints[SideBack].x;
	}



	void FaceGeometry::toFile(const std::string& fn)
	{
		std::ofstream f(fn.c_str());

		// order (must be the same in both programs!): leftEye, rightEye, nose, mouth, chin
		pointToFile(f, leftEye);
		pointToFile(f, rightEye);
		pointToFile(f, nose);
		pointToFile(f, mouth);
		pointToFile(f, chin);
		pointToFile(f, faceDimensions);
		pointToFile(f, m_DetectedPoints[TextureLeftEye]);
		pointToFile(f, m_DetectedPoints[TextureRightEye]);
		pointToFile(f, m_DetectedPoints[TextureChin]);
	}


	
	void FaceGeometry::pointToFile(std::ofstream& f, const cv::Point3d& p)
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