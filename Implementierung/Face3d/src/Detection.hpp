#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "FaceGeometry.hpp"

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

		// extract geomtry of face from images
		struct DetectFaceResult
		{
			FaceGeometry faceGeometry;
			cv::Mat textureFront;
			cv::Mat textureSide;
		};
		DetectFaceResult detectFace();

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


		// helper function to get original image to draw on for debug output
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
		void doMatchCoordinates();
		void createTextures();
		cv::Rect getBoundingBox(const cv::Mat& color);


		// original images, stored in array to help generalize some of the algorithms (by just iterating over the elements)
		const size_t frontImgNr = 0;
		const size_t sideImgNr = 1;
		const size_t imgSize = 320;
		std::vector<cv::Mat> m_Originals;
		std::vector<cv::Mat> m_Preprocessed;
		std::vector<cv::Mat> m_FaceExtracted;
		std::vector<cv::Mat> m_FaceMask;
		FaceGeometry m_FaceGeometry;
		std::vector<cv::Mat> m_Textures;


		

	};
}