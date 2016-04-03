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
		doFacialComponentsExtraction();
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

			// do some morphological erode (enlarges black regions)
			// REMARK: this is not in the original paper but helps to find the facial components 
			cv::Mat structElement = cv::getStructuringElement
			(
				cv::MORPH_RECT
				, cv::Size(5, 5)
				, cv::Point(1, 1)
			);

			//cv::dilate(combinedThres, combinedThres, structElement);
			cv::erode(combinedThres, combinedThres, structElement);		
			
			// add to result
			m_FaceExtracted.push_back(combinedThres);
		}

	
		dbgShow(m_FaceExtracted[0], "m_FaceExtracted");

		cv::Mat tmp1(m_Originals[0]),tmp2;
		tmp1.copyTo(tmp2, m_FaceExtracted[0]);
		dbgShow(tmp2, "m_FaceExtracted & m_Originals");
	
	}



	void Detection::doFacialComponentsExtraction()
	{
		/*
		REMARK: as the original paper does not say too much how specific components are found, we will implement this step according to Akimoto:
		the inner regions (the black holes in the white face) are used and are classified according to some very simple rules:
		-> size
		-> position (relative to center of gravity of face) 
		*/

		// find the contours (bounded binary regions)
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(m_FaceExtracted[0], contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		// indices of the potential face components
		std::vector<size_t> potentialFacialComponents;


		// temporary image for debug output
		cv::Mat tmp;
		m_Originals[0].copyTo(tmp);

		assert(contours.size() == hierarchy.size()); // should be the same according to opencv doc!
		for (size_t i = 0; i < contours.size(); ++i)
		{
			/*
			from http://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=findcontours#findcontours:
			hierarchy – Optional output vector, containing information about the image topology. It has as many elements as the number of contours.
			For each i-th contour contours[i] , the elements hierarchy[i][0] , hiearchy[i][1] , hiearchy[i][2] , and hiearchy[i][3]
			are set to 0-based indices in contours of the next and previous contours at the same hierarchical level, the first child contour and the parent contour,
			respectively. If for the contour i there are no next, previous, parent, or nested contours, the corresponding elements of hierarchy[i] will be negative.
			*/
			const int NEXT_CONTOUR = 0, PREV_CONTOUR = 1, CHILD_CONTOUR = 2, PARENT_CONTOUR = 3;

			// draw just those contours which are nestet, i.e. which have a parent contour!
			if (hierarchy[i][PARENT_CONTOUR] >= 0)
			{
				//cv::drawContours(tmp, contours, i, cv::Scalar(255, 0, 0), -1);
				potentialFacialComponents.push_back(i);
			}
		}


		// debug draw
		for (size_t i = 0; i < potentialFacialComponents.size(); ++i)
		{
			cv::drawContours(tmp, contours, potentialFacialComponents[i], cv::Scalar(255, 0, 0), -1);
		}
		dbgShow(tmp,"all potential facial components");


		// let's do some really simple classification:
		// we take the 3 biggest regions, we say that those are: left & right eye and mouth
		// now we look at the position relative to the center of gravity to assign labels to them

		
		// functor such that STL can be used to sort contours according to area
		struct ContourArea
		{
			ContourArea() : indexOfContour(0),areaOfContour(0.0){}
			size_t indexOfContour;
			double areaOfContour;

			bool operator<(const ContourArea& other) 
			{
				return areaOfContour>other.areaOfContour;
			}
		};


		std::vector<ContourArea> contourAreas;
		for (size_t i = 0; i < potentialFacialComponents.size(); ++i)
		{			
			ContourArea contourArea;
			contourArea.indexOfContour = potentialFacialComponents[i];
			contourArea.areaOfContour = cv::contourArea(contours[potentialFacialComponents[i]]);									

			contourAreas.push_back(contourArea);
		}

		std::sort(contourAreas.begin(), contourAreas.end());

		// we need at least 3 elements (left & right eye, mouth)
		if (contourAreas.size()<3)
		{
			throw std::exception("we need at least 3 regions for classification as left & right eye, mouth");
		}

		contourAreas.erase(contourAreas.begin()+3, contourAreas.end());


		// debug draw		
		m_Originals[0].copyTo(tmp);
		for (size_t i = 0; i < contourAreas.size(); ++i)
		{
			cv::drawContours(tmp, contours, contourAreas[i].indexOfContour, cv::Scalar(0, 0, 255), -1);
		}
		dbgShow(tmp, "left and right eye, mouth");


	}

}