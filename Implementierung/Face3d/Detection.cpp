#include "Detection.hpp"
#include "Common.hpp"

namespace Face3D
{


	//-------------------------------------------------------------------------
	// CTOR: save original images at a specific size (currently at 320x320)
	Detection::Detection(const cv::Mat& front, const cv::Mat& side)
	{		
		/*
		we expect the images to be square (e.g. 640x640)
		we will resize the input images to a fixed size of 320x320
		*/

		m_Originals.resize(2);
		
		assert(front.size().width == front.size().height);
		cv::resize(front, m_Originals[frontImgNr], cv::Size(320, 320));

		assert(side.size().width == side.size().height);
		cv::resize(side, m_Originals[sideImgNr], cv::Size(320, 320));		
	}



	//-------------------------------------------------------------------------
	// execute all steps
	Detection::DetectionResult Detection::doAllSteps()
	{
		doPreprocessing();		
		doFaceExtraction();
		doFacialComponentsExtraction();
		// ...


		return DetectionResult();
	}



	//-------------------------------------------------------------------------
	// preprocess image: smoothing, ...)
	void Detection::doPreprocessing()
	{
		m_Preprocessed.resize(2);
		for (size_t i = 0; i < m_Originals.size(); ++i)
		{
			cv::Mat resizedImg;
			cv::GaussianBlur(m_Originals[i], m_Preprocessed[i], cv::Size(5, 5), 0, 0);
		}		
	}



	//-------------------------------------------------------------------------
	// extract binary regions representing the face and its components (nose, eyes, ...)
	void Detection::doFaceExtraction()
	{
		for (size_t i = 0; i < m_Preprocessed.size(); ++i)
		{
			// to YCrCb colorspace
			cv::Mat ycrcb;
			cv::cvtColor(m_Preprocessed[i], ycrcb, CV_BGR2YCrCb);

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

			dbgShow(m_FaceExtracted[i], "doFaceExtraction",i);
		}
	}



	//-------------------------------------------------------------------------
	// extract facial components from the binary regions
	void Detection::doFacialComponentsExtraction()
	{
		/*
		REMARK: as the original paper does not say too much how specific components are found, we will implement this step according to Akimoto:
		the inner regions (the black holes in the white face) are used and are classified according to some very simple rules:
		-> size
		-> position (relative to center of gravity of face) 
		*/
		
		for (size_t i = 0; i < m_FaceExtracted.size(); ++i)
		{
			// find the contours (bounded binary regions)
			std::vector<std::vector<cv::Point> > contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::findContours(m_FaceExtracted[i], contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

			// indices of the potential face components
			std::vector<size_t> potentialComponentIndices = findRegions(contours,hierarchy,RegionTypeInside); // indices of facial components
			std::vector<size_t> potentialFaceIndices = findRegions(contours, hierarchy, RegionTypeOutside); // indices of face (skin) region

			// extract information for those regions
			std::vector<ContourInfo> potentialComponentContourInfo = extractContourInfo(contours, potentialComponentIndices); // facial components
			std::vector<ContourInfo> potentialFaceContourInfo = extractContourInfo(contours, potentialFaceIndices); // face (skin) region			

			if (frontImgNr==i)
			{
				doFacialComponentsExtractionFront(m_FaceGeometry, potentialComponentContourInfo, potentialFaceContourInfo);				
			}
			else if (sideImgNr==i)
			{
				doFacialComponentsExtractionSide(m_FaceGeometry, potentialComponentContourInfo, potentialFaceContourInfo);
			}						
		}


		// draw resulting 2d centroids
		// 1. front
		cv::Mat tmp = getCopyOfOriginal(frontImgNr);
		cv::circle(tmp, cv::Point(m_FaceGeometry.frontLeftEye), 10, cv::Scalar(255, 0, 0), 2);
		cv::circle(tmp, cv::Point(m_FaceGeometry.frontRightEye), 10, cv::Scalar(0, 255, 0), 2);
		cv::circle(tmp, cv::Point(m_FaceGeometry.frontMouth), 10, cv::Scalar(0, 0, 255), 2);
		dbgShow(tmp,"doFacialComponentsExtraction",0);

		// 2. side
		tmp = getCopyOfOriginal(sideImgNr);
		cv::circle(tmp, cv::Point(m_FaceGeometry.sideEye), 10, cv::Scalar(255, 0, 0), 2);
		cv::circle(tmp, cv::Point(m_FaceGeometry.sideNoseTip), 10, cv::Scalar(255, 0, 255), 2);
		dbgShow(tmp,"doFacialComponentsExtraction",1);

	}



	//-------------------------------------------------------------------------
	// extract facial components (front image) from the binary regions
	void Detection::doFacialComponentsExtractionFront(FaceGeometry& faceGeometry, const std::vector<ContourInfo>& componentContourInfo, const std::vector<ContourInfo>& faceContourInfo)
	{
		// we need at least 3 elements (left & right eye, mouth)
		if (componentContourInfo.size()<3 || faceContourInfo.size()<1)
		{
			throw std::exception("we need at least 3 regions for classification as left & right eye, mouth (front image)");
		}
		
		std::vector<ContourInfo> biggestThree(componentContourInfo.begin(), componentContourInfo.begin() + 3);



		ContourInfo mouth;
		std::vector<ContourInfo> eyes;
		for (size_t i = 0; i < biggestThree.size(); ++i)
		{
			if (biggestThree[i].cogY > faceContourInfo[0].cogY)
			{
				mouth = biggestThree[i];
			}
			else
			{
				eyes.push_back(biggestThree[i]);
			}
		}

		// left / right eye
		ContourInfo leftEye = eyes[0].cogX < eyes[1].cogX ? eyes[0] : eyes[1];
		ContourInfo rightEye = eyes[0].cogX > eyes[1].cogX ? eyes[0] : eyes[1];

		faceGeometry.frontLeftEye = cv::Point2d(leftEye.cogX, leftEye.cogY);
		faceGeometry.frontRightEye = cv::Point2d(rightEye.cogX, rightEye.cogY);
		faceGeometry.frontMouth = cv::Point2d(mouth.cogX, mouth.cogY);
		
		cv::Mat tmp=getCopyOfOriginal(frontImgNr);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {leftEye.contour}, 0, cv::Scalar(255, 0, 0), -1);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {rightEye.contour}, 0, cv::Scalar(0, 255, 0), -1);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {mouth.contour}, 0, cv::Scalar(0, 0, 255), -1);			
		dbgShow(tmp, "doFacialComponentsExtractionFront");
	}



	//-------------------------------------------------------------------------
	// extract facial components (side image) from the binary regions
	void Detection::doFacialComponentsExtractionSide(FaceGeometry& faceGeometry, const std::vector<ContourInfo>& componentContourInfo, const std::vector<ContourInfo>& faceContourInfo)
	{
		// we need at least 3 elements (left & right eye, mouth)
		if (componentContourInfo.size()<1 || faceContourInfo.size()<1)
		{
			throw std::exception("we need at least 1 region for classification as eye (side image)");
		}

		ContourInfo eye = componentContourInfo[0];
		ContourInfo face = faceContourInfo[0];

		faceGeometry.sideEye = cv::Point2d(eye.cogX, eye.cogY);
		
		faceGeometry.sideNoseTip = cv::Point2d(0,0);
		for (size_t i = 0; i < face.contour.size(); ++i)
		{
			if (face.contour[i].x > faceGeometry.sideNoseTip.x)
			{
				faceGeometry.sideNoseTip = face.contour[i];
			}
		}
		


		cv::Mat tmp = getCopyOfOriginal(sideImgNr);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {eye.contour}, 0, cv::Scalar(255, 0, 0), -1);
		dbgShow(tmp, "doFacialComponentsExtractionSide");
	}



	//-------------------------------------------------------------------------
	std::vector<size_t> Detection::findRegions(const std::vector<std::vector<cv::Point> >& contours, const std::vector<cv::Vec4i>& hierarchy, RegionType regionType)
	{
		/*
		from http://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=findcontours#findcontours:
		hierarchy – Optional output vector, containing information about the image topology. It has as many elements as the number of contours.
		For each i-th contour contours[i] , the elements hierarchy[i][0] , hiearchy[i][1] , hiearchy[i][2] , and hiearchy[i][3]
		are set to 0-based indices in contours of the next and previous contours at the same hierarchical level, the first child contour and the parent contour,
		respectively. If for the contour i there are no next, previous, parent, or nested contours, the corresponding elements of hierarchy[i] will be negative.
		*/
		const int NEXT_CONTOUR = 0, PREV_CONTOUR = 1, CHILD_CONTOUR = 2, PARENT_CONTOUR = 3;				

		std::vector<size_t> res;

		assert(contours.size() == hierarchy.size()); // should be the same according to opencv doc!
		for (size_t i = 0; i < contours.size(); ++i)
		{
			// take just those contours which are nestet, i.e. which have a parent contour!
			if ((regionType == RegionTypeInside && hierarchy[i][PARENT_CONTOUR]>= 0) || (regionType == RegionTypeOutside && hierarchy[i][PARENT_CONTOUR] < 0))
			{				
				res.push_back(i);
			}
		}

		return res;
	}



	//-------------------------------------------------------------------------
	std::vector<Detection::ContourInfo> Detection::extractContourInfo(const std::vector<std::vector<cv::Point> >& contours, const std::vector<size_t> indices)
	{
		std::vector<ContourInfo> contourInfos;

		for (size_t i = 0; i < indices.size(); ++i)
		{
			ContourInfo contourInfo;
			contourInfo.contour = contours[indices[i]];
			contourInfo.areaOfContour = cv::contourArea(contours[indices[i]]);

			cv::Moments moments = cv::moments(contours[indices[i]]);
			contourInfo.cogX = moments.m10 / moments.m00;
			contourInfo.cogY = moments.m01 / moments.m00;

			contourInfos.push_back(contourInfo);
		}

		// sort by area
		std::sort(contourInfos.begin(), contourInfos.end());

		return contourInfos;
	}



}