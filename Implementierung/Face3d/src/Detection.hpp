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

	void onColorThresholdsTrackbar(int, void*);
	void onTextureAdjustmentTrackbar(int val, void* ptr);
	bool isConcave(const cv::Point& a, const cv::Point& b, const cv::Point& c);

	/** class which detects the facial components and calculates the face geometry */
	class Detection
	{
	public:		

		/** \brief  create a Detection object
		* \param front image of the face from the front
		* \param side image of the face from the side
		*/
		Detection(const cv::Mat& front, const cv::Mat& side);

		/** class which holds the result: information about the face geometry and the front and side textures */
		struct DetectFaceResult
		{
			FaceGeometry faceGeometry;
			cv::Mat textureFront;
			cv::Mat textureSide;
		};

		/** \brief  calculate the face geometry and the textures
		* \return a DetectFaceResult object which holds the face geometry and the textures
		*/
		DetectFaceResult detectFace();

	private:
		/** functor such that STL can be used to sort contours according to area */
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


		/** helper function to get original image to draw on for debug output */
		cv::Mat getCopyOfOriginal(int imgNr)
		{
			cv::Mat tmp;
			m_Originals[imgNr].copyTo(tmp);
			return tmp;
		}

		/** preprocessing: smooth image */
		void doPreprocessing();
		
		/** extracts face (skin) region */
		void doFaceExtraction();

		/** extract facial components (eyes, nose, ...) */
		void doFacialComponentsExtraction();

		/** extract facial components in front image */
		void doFacialComponentsExtractionFront(FaceGeometry& faceGeometry, const std::vector<ContourInfo>& componentContourInfo, const std::vector<ContourInfo>& faceContourInfo);

		/** extract facial components in side image */
		void doFacialComponentsExtractionSide(FaceGeometry& faceGeometry, const std::vector<ContourInfo>& componentContourInfo, const std::vector<ContourInfo>& faceContourInfo);

		/** type of binary region to be looked for: an inner or an outer region */
		enum RegionType{ RegionTypeInside, RegionTypeOutside };

		/** fget list of regions with specified type */
		std::vector<size_t> findRegions(const std::vector<std::vector<cv::Point> >& contours, const std::vector<cv::Vec4i>& hierarchy, RegionType regionType);

		/** calculate informations about the contours, e.g. the area */
		std::vector<ContourInfo> extractContourInfo(const std::vector<std::vector<cv::Point> >& contours, const std::vector<size_t> indices);
		
		/** match the 2d coordinates of the facial components in the two images to get the 3d information */
		void doMatchCoordinates();

		/** create the textures from the two images and align them */
		void createTextures();

		/** get the bounding box of the color image. background must be black, everything else will be regared as foreground. */
		cv::Rect getBoundingBox(const cv::Mat& color);

		/** gui interaction*/
		int m_ColorThresValue = 10;
		int m_OffsetCB=0, m_OffsetCR=0;
		double m_AddTexture = 0;
		void doFaceExtractionGUI();
		void createTexturesAndShowResultsGUI();
		cv::Mat combineVertically(const cv::Mat& a, const cv::Mat& b) const;


		/** images (originals and processed) */
		const size_t frontImgNr = 0; ///< index of the front image when both images are stored in an array
		const size_t sideImgNr = 1; ///< index of the side image when both images are stored in an array
		const size_t imgSize = 320; ///< size of the images we are working with. 320x320 seems good as its fast but has still enough details
		std::vector<cv::Mat> m_Originals; ///< original images (scaled down)
		std::vector<cv::Mat> m_Preprocessed; ///< preprocessed images (smooth)
		std::vector<cv::Mat> m_FaceExtracted; ///< binary image with skin as foreground
		std::vector<cv::Mat> m_FaceMask; ///< mask of the face regions. foreground regions which are not the face are already removed
		FaceGeometry m_FaceGeometry, m_FaceGeometryBackup; ///< the geometry of the face, i.e. the coordinates of the facial components
		std::vector<cv::Mat> m_Textures; ///< the textures, already in a format that can be used in OpenGL
		
		/** resulting images to show in the gui */
		cv::Mat m_FacialPointsGUI;
		cv::Mat m_TexturesGUI;


		friend void onColorThresholdsTrackbar(int, void*);
		friend void onTextureAdjustmentTrackbar(int, void*);
	};
}