#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

/*
This class implements the first part of the pileline: detection of the individual face components
According to the paper, the following steps have to be executed:
1. Preprocessing
2. Extract face region
3. Extract facial components
*/

namespace Face3D
{
	class Detection
	{
	public:
		// CTOR
		Detection(const cv::Mat& front, const cv::Mat& side);

		// execute all steps
		struct DetectionResult
		{
			// TODO: result of all the steps should be the 2d or 3d coordintates (!?)
		};
		DetectionResult doAllSteps();




	private:
		// functor such that STL can be used to sort contours according to area
		struct ContourInfo
		{
			ContourInfo() : areaOfContour(0.0), cogX(0.0), cogY(0.0){}
			std::vector<cv::Point> contour;
			double areaOfContour;
			double cogX, cogY;

			bool operator<(const ContourInfo& other)
			{
				return areaOfContour>other.areaOfContour;
			}
		};



		struct FaceGeometry
		{
			// 2d position of components in front image
			cv::Point2d frontLeftEye, frontRightEye, frontMouth;

			// 2d position of components in side image
			cv::Point2d sideEye,sideNoseTip;

			// combine the points into 3d points
			void merge(){/*TODO*/ }
		};

		cv::Mat getCopyOfOriginal(int imgNr)
		{
			cv::Mat tmp;
			m_Originals[imgNr].copyTo(tmp);
			return tmp;
		}


		void doPreprocessing();
		void doFaceExtraction();
		void doFacialComponentsExtraction();
		void doFacialComponentsExtractionFront(FaceGeometry& faceGeometry, const std::vector<ContourInfo>& componentContourInfo, const std::vector<ContourInfo>& faceContourInfo);
		void doFacialComponentsExtractionSide(FaceGeometry& faceGeometry, const std::vector<ContourInfo>& componentContourInfo, const std::vector<ContourInfo>& faceContourInfo);
		enum RegionType{ RegionTypeInside, RegionTypeOutside };
		std::vector<size_t> findRegions(const std::vector<std::vector<cv::Point> >& contours, const std::vector<cv::Vec4i>& hierarchy, RegionType regionType);
		std::vector<ContourInfo> extractContourInfo(const std::vector<std::vector<cv::Point> >& contours, const std::vector<size_t> indices);


		// original images, stored in array to help generalize some of the algorithms (by just iterating over the elements)
		const size_t frontImgNr = 0;
		const size_t sideImgNr = 1;
		const size_t minContourSize = 10;
		std::vector<cv::Mat> m_Originals;
		std::vector<cv::Mat> m_Preprocessed;
		std::vector<cv::Mat> m_FaceExtracted;
		FaceGeometry m_FaceGeometry;


		

	};
}