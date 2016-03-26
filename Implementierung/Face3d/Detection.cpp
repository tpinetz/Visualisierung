#include "Detection.hpp"
#include "Common.hpp"

namespace Face3D
{

	Detection::Detection(const cv::Mat& front, const cv::Mat& side)
	{
		m_Originals.push_back(front);
		m_Originals.push_back(side);
	}

	// execute all steps
	Detection::DetectionResult Detection::doAllSteps()
	{
		doPreprocessing();		
		doFaceExtraction();
		// ...


		return DetectionResult();
	}

	void Detection::doPreprocessing()
	{
		for (size_t i = 0; i < m_Originals.size(); ++i)
		{
			cv::Mat resizedImg;
			cv::resize(m_Originals[i], resizedImg, cv::Size(), 1, 1); // REMARK: no rescaling at the moment
			m_Preprocessed.push_back(resizedImg);
		}
		
		dbgShow(m_Preprocessed[0],"m_Preprocessed");
	}

	void Detection::doFaceExtraction()
	{
		for (size_t i = 0; i < m_Preprocessed.size(); ++i)
		{
			// to YCrCb colorspace
			cv::Mat ycrcb;
			cv::cvtColor(m_Preprocessed[0], ycrcb, CV_BGR2YCrCb);

			// split color channels into seperate grayscale images
			std::vector<cv::Mat> channels;
			cv::split(ycrcb, channels);

			// threshold cr and cb color channel
			cv::Mat crThres, cbThres;
			cv::inRange(channels[1], cv::Scalar(143), cv::Scalar(173), crThres);
			cv::inRange(channels[2], cv::Scalar(77), cv::Scalar(125), cbThres);		 // REMARK: tweaked the values a bit to get better results

			// combine result with bitwise and
			cv::Mat combinedThres;
			cv::bitwise_and(crThres, cbThres, combinedThres);
			
			// add to result
			m_FaceExtracted.push_back(combinedThres);
		}

	
		dbgShow(m_FaceExtracted[0], "m_FaceExtracted");

		cv::Mat tmp1(m_Originals[0]),tmp2;
		tmp1.copyTo(tmp2, m_FaceExtracted[0]);
		dbgShow(tmp2, "m_FaceExtracted & m_Originals");

		/*
		TODO: hier weitermachen (größte Kontur finden, das ist die Haut, diese abziehen, es verbleiben Augen und Mund und Störungen. Augen und Mund erkennen
		*/


		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(m_FaceExtracted[0], contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		cv::Mat contourImg = cv::Mat(m_Preprocessed[0].rows, m_Preprocessed[0].cols, CV_8UC1, cv::Scalar(0));
		for (auto contour : contours) {
			if (contour.size() > this->minContourSize) {
				for (auto point : contour) {
					contourImg.at<uchar>(point) = 255;
				}
			}
		}

		dbgShow(contourImg);
		

	}

}