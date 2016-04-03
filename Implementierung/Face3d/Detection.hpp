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
		void doPreprocessing();
		void doFaceExtraction();
		void doFacialComponentsExtraction();


		// original images, stored in array to help generalize some of the algorithms (by just iterating over the elements)
		const size_t frontImgNr = 0;
		const size_t sideImgNr = 1;
		const size_t minContourSize = 10;
		std::vector<cv::Mat> m_Originals;
		std::vector<cv::Mat> m_Preprocessed;
		std::vector<cv::Mat> m_FaceExtracted;

	};
}