#include "Detection.hpp"
#include "Common.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctime>


namespace Face3D
{


	Detection::Detection(const cv::Mat& front, const cv::Mat& side)
	{		
		/*
		we expect the images to be square (e.g. 640x640)
		we will resize the input images to a fixed size of 320x320
		*/

		m_Originals.resize(2);
		
		assert(front.size().width == front.size().height);
		cv::resize(front, m_Originals[frontImgNr], cv::Size(imgSize, imgSize));

		assert(side.size().width == side.size().height);
		cv::resize(side, m_Originals[sideImgNr], cv::Size(imgSize, imgSize));
	}



	Detection::DetectFaceResult Detection::detectFace()
	{
		// execute the pipeline
		doPreprocessing();	
		doFaceExtractionGUI();
		doFacialComponentsExtraction();
		doMatchCoordinates();				
		createTexturesAndShowResultsGUI();

		// prepare result and return it
		DetectFaceResult res;
		res.faceGeometry = m_FaceGeometry;
		res.textureFront = m_Textures[frontImgNr];
		res.textureSide = m_Textures[sideImgNr];
		return res;
	}



	void Detection::doPreprocessing()
	{
		m_Preprocessed.resize(2);
		for (size_t i = 0; i < m_Originals.size(); ++i)
		{
			cv::Mat resizedImg;
			cv::GaussianBlur(m_Originals[i], m_Preprocessed[i], cv::Size(5, 5), 0, 0);
		}		
	}


	void Detection::doFaceExtraction()
	{
		// clear previous result because this function could be called multiple times from the gui
		m_FaceExtracted.clear();

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
			cv::inRange(channels[1], cv::Scalar(143 + m_OffsetCR), cv::Scalar(173 + m_OffsetCR), crThres);
			cv::inRange(channels[2], cv::Scalar(77 + m_OffsetCB), cv::Scalar(125 + m_OffsetCB), cbThres);		 // REMARK: tweaked the values a bit to get better results

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

			// dbgShow(m_FaceExtracted[i], "doFaceExtraction",i); this is already shown in the gui
		}
	}



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

			// copy original as findContours changes image
			cv::Mat tmp; 
			m_FaceExtracted[i].copyTo(tmp);

			cv::findContours(tmp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

			// indices of the potential face components
			std::vector<size_t> potentialComponentIndices = findRegions(contours,hierarchy,RegionTypeInside); // indices of facial components
			std::vector<size_t> potentialSkinIndices = findRegions(contours, hierarchy, RegionTypeOutside); // indices of face (skin) region

			// extract information for those regions
			std::vector<ContourInfo> potentialComponentContourInfo = extractContourInfo(contours, potentialComponentIndices); // facial components
			std::vector<ContourInfo> potentialSkinContourInfo = extractContourInfo(contours, potentialSkinIndices); // face (skin) region			

			if (frontImgNr==i)
			{
				doFacialComponentsExtractionFront(m_FaceGeometry, potentialComponentContourInfo, potentialSkinContourInfo);
			}
			else if (sideImgNr==i)
			{
				doFacialComponentsExtractionSide(m_FaceGeometry, potentialComponentContourInfo, potentialSkinContourInfo);
			}						
		}




		// draw resulting 2d centroids
		// 1. front
		cv::Mat tmp1 = getCopyOfOriginal(frontImgNr);
		cv::circle(tmp1, m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontLeftEye), 10, cv::Scalar(255, 0, 0), 2);
		cv::circle(tmp1, m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontRightEye), 10, cv::Scalar(0, 255, 0), 2);
		cv::circle(tmp1, m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontMouth), 10, cv::Scalar(0, 0, 255), 2);
		cv::circle(tmp1, m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontLeftCheek), 10, cv::Scalar(255, 255, 255), 2);
		cv::circle(tmp1, m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontRightCheek), 10, cv::Scalar(255, 255, 255), 2);
		dbgShow(tmp1,"doFacialComponentsExtraction",0);

		// 2. side
		cv::Mat tmp2 = getCopyOfOriginal(sideImgNr);
		cv::circle(tmp2, m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideEye), 10, cv::Scalar(255, 0, 0), 2);
		cv::circle(tmp2, m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideNoseTip), 10, cv::Scalar(255, 0, 255), 2);
		cv::circle(tmp2, m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideChin), 10, cv::Scalar(255, 255, 0), 2);
		cv::circle(tmp2, m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideBack), 10, cv::Scalar(255, 255, 255), 2);
		dbgShow(tmp2,"doFacialComponentsExtraction",1);

		// put into result image for gui
		m_FacialPointsGUI = combineVertically(tmp1,tmp2);

	}


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

		// we need at least 3 elements (left & right eye, mouth)
		if (eyes.size()!=2)
		{
			throw std::exception("couldn't identify both eyes");
		}


		// left / right eye
		ContourInfo leftEye = eyes[0].cogX < eyes[1].cogX ? eyes[0] : eyes[1];
		ContourInfo rightEye = eyes[0].cogX > eyes[1].cogX ? eyes[0] : eyes[1];

		faceGeometry.setDetectedPoint(FaceGeometry::FrontLeftEye, cv::Point2d(leftEye.cogX, leftEye.cogY));
		faceGeometry.setDetectedPoint(FaceGeometry::FrontRightEye, cv::Point2d(rightEye.cogX, rightEye.cogY));
		faceGeometry.setDetectedPoint(FaceGeometry::FrontMouth, cv::Point2d(mouth.cogX, mouth.cogY));		



		// find sides of face (xmin, xmax)
		const cv::Point eyePos = faceGeometry.getDetectedPoint(FaceGeometry::FrontLeftEye);
		cv::Point leftCheek, rightCheek;

		// left
		for (int x = 0; x<m_FaceExtracted[sideImgNr].cols; ++x)
		{
			if (m_FaceExtracted[frontImgNr].at<unsigned char>(cv::Point(x, eyePos.y)) != 0)
			{
				leftCheek.x = x;
				leftCheek.y = eyePos.y;
				break;
			}
		}

		// right 
		for (int x = m_FaceExtracted[sideImgNr].cols-1; x>=0; --x)
		{
			if (m_FaceExtracted[frontImgNr].at<unsigned char>(cv::Point(x, eyePos.y)) != 0)
			{
				rightCheek.x = x;
				rightCheek.y = eyePos.y;
				break;
			}
		}

		faceGeometry.setDetectedPoint(FaceGeometry::FrontLeftCheek, leftCheek);
		faceGeometry.setDetectedPoint(FaceGeometry::FrontRightCheek, rightCheek);
		

		// create face mask
		cv::Mat mask(imgSize,imgSize,CV_8U);
		mask.setTo(0);
		cv::drawContours(mask, std::vector<std::vector<cv::Point> > {faceContourInfo[0].contour}, 0, 255, -1);
		m_FaceMask.push_back(mask);		

		
		// show debug info
		cv::Mat tmp=getCopyOfOriginal(frontImgNr);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {leftEye.contour}, 0, cv::Scalar(255, 0, 0), -1);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {rightEye.contour}, 0, cv::Scalar(0, 255, 0), -1);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {mouth.contour}, 0, cv::Scalar(0, 0, 255), -1);			
		dbgShow(tmp, "doFacialComponentsExtractionFront");
	}


	bool isConcave(const cv::Point& a, const cv::Point& b, const cv::Point& c)
	{
		// calc two 2d-vectors 
		int v1x = b.x - a.x;
		int v1y = b.y - a.y;

		int v2x = c.x - a.x;
		int v2y = c.y - a.y;

		// det(v1,v2) determines if this is a convex or concave part of the polygon
		int det = v1x*v2y - v1y*v2x;

		return det < 0;
	}


	void Detection::doFacialComponentsExtractionSide(FaceGeometry& faceGeometry, const std::vector<ContourInfo>& componentContourInfo, const std::vector<ContourInfo>& faceContourInfo)
	{
		// we need at least 3 elements (left & right eye, mouth)
		if (componentContourInfo.size()<1 || faceContourInfo.size()<1)
		{
			throw std::exception("we need at least 1 region for classification as eye (side image)");
		}

		const ContourInfo& eye = componentContourInfo[0];
		const ContourInfo& face = faceContourInfo[0];
		
		faceGeometry.setDetectedPoint(FaceGeometry::SideEye, cv::Point2d(eye.cogX, eye.cogY));
		
						
		// find bounding polygon with at least 5 vertices
		std::vector<cv::Point> polygonPoints;
		double precission = 50.0;
		while (polygonPoints.size() < 5)
		{				
			cv::approxPolyDP(face.contour, polygonPoints, precission, true);
			precission = precission / 2;
		}
			
		

		// nose tip is rightmost point (of this polygon)		
		size_t noseIdx = 0;
		faceGeometry.setDetectedPoint(FaceGeometry::SideNoseTip, cv::Point2d(0, 0));
		for (size_t i = 0; i < polygonPoints.size(); ++i)
		{
			if (polygonPoints[i].x > faceGeometry.getDetectedPoint(FaceGeometry::SideNoseTip).x)
			{
				faceGeometry.setDetectedPoint(FaceGeometry::SideNoseTip, polygonPoints[i]);
				noseIdx = i;
			}
		}


		// find chin: which direction must be searched for in the polygon?
		bool incIdx = false;
		const size_t numPolygonPoints = polygonPoints.size();
		if (polygonPoints[(noseIdx + 1) % numPolygonPoints].y > faceGeometry.getDetectedPoint(FaceGeometry::SideNoseTip).y)
		{
			incIdx = true;
		}

		// search for the following pattern: find a convex and a neighboured concave polygon part under the nose
		faceGeometry.setDetectedPoint(FaceGeometry::SideChin, cv::Point2d(0, 0));
		for (size_t i = noseIdx, j = 0; j<numPolygonPoints; incIdx ? ++i : --i, ++j)
		{
			// don't check the nose itself
			if (j==0)
			{
				continue;
			}
				
			// get prev, curr and next point
			cv::Point prevprev = polygonPoints[(incIdx ? (i - 2) : (i + 2)) % numPolygonPoints];
			cv::Point prev = polygonPoints[(incIdx ? (i - 1) : (i + 1)) % numPolygonPoints];
			cv::Point curr = polygonPoints[i % numPolygonPoints];
			cv::Point next = polygonPoints[(incIdx ? (i + 1) : (i - 1)) % numPolygonPoints];

			if (isConcave(prev, curr, next) && !isConcave(prevprev, prev, curr))
			{
				faceGeometry.setDetectedPoint(FaceGeometry::SideChin, prev);
				break;
			}
		}					


		// find back side of head
		const cv::Point chinPoint = faceGeometry.getDetectedPoint(FaceGeometry::SideChin);
		cv::Point backPoint;
		for (int x = 0; x<m_FaceExtracted[sideImgNr].cols; ++x)
		{			
			if (m_FaceExtracted[sideImgNr].at<unsigned char>(cv::Point(x, chinPoint.y))!=0)
			{
				backPoint.x = x;
				backPoint.y = chinPoint.y;
				break;
			}
		}
		faceGeometry.setDetectedPoint(FaceGeometry::SideBack, backPoint);


		// create face mask
		cv::Mat mask(imgSize, imgSize, CV_8U);
		mask.setTo(0);
		cv::drawContours(mask, std::vector<std::vector<cv::Point> > {face.contour}, 0, 255, -1);
		m_FaceMask.push_back(mask);
		

		// show debug info
		cv::Mat tmp = getCopyOfOriginal(sideImgNr);
		cv::drawContours(tmp, std::vector<std::vector<cv::Point> > {eye.contour}, 0, cv::Scalar(255, 0, 0), -1);
		dbgShow(tmp, "doFacialComponentsExtractionSide");

		// and the polygon
		cv::Mat tmpChin = cv::Mat::zeros(imgSize, imgSize, CV_8UC3);
		cv::drawContours(tmpChin, std::vector<std::vector<cv::Point> > {face.contour}, 0, cv::Scalar(100, 100, 100), -1);
		cv::RNG rng(0);
		for (size_t i = 0; i < numPolygonPoints; ++i)
		{
			std::cout << "Point: " << polygonPoints[i] << "\n";
			cv::line(tmpChin, polygonPoints[i], polygonPoints[(i + 1) % numPolygonPoints], cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 5);
		}

		dbgShow(tmpChin, "doFacialComponentsExtractionSide");
	}




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



	void Detection::doMatchCoordinates()
	{
		m_FaceGeometry.merge3d();
	}

	

	void Detection::createTextures()
	{						
		// allocate images		
		m_Textures.resize(2);
		
		// zero-out images
		m_Textures[frontImgNr] = cv::Mat::zeros(imgSize,imgSize,CV_8UC3);
		m_Textures[sideImgNr] = cv::Mat::zeros(imgSize, imgSize, CV_8UC3);

		// mask images
		m_Originals[frontImgNr].copyTo(m_Textures[frontImgNr], m_Originals[frontImgNr]);
		m_Originals[sideImgNr].copyTo(m_Textures[sideImgNr], m_Originals[sideImgNr]);		

		//dbgShow(m_Textures[frontImgNr], "TestImage");

		// front image: rotate so that eyes are on a horizontal line
		cv::Vec2d vecEyes(m_FaceGeometry.getDetectedPoint(FaceGeometry::FrontRightEye) - m_FaceGeometry.getDetectedPoint(FaceGeometry::FrontLeftEye));
		const double alpha=atan(vecEyes[1] / vecEyes[0])*180.0/M_PI;
		cv::Point centerPoint(m_Textures[frontImgNr].cols / 2, m_Textures[frontImgNr].rows / 2);

		// transform front image
		cv::Mat rotMat = cv::getRotationMatrix2D(centerPoint, alpha, 1.0);
		cv::warpAffine(m_Textures[frontImgNr], m_Textures[frontImgNr], rotMat, m_Textures[frontImgNr].size());
		

		// transform all front coordinates
		m_FaceGeometry.transform(FaceGeometry::FrontLeftEye, rotMat);
		m_FaceGeometry.transform(FaceGeometry::FrontRightEye, rotMat);
		m_FaceGeometry.transform(FaceGeometry::FrontMouth, rotMat);
		
		//dbgShow(m_Textures[frontImgNr], "createTextures: frontImage rotated");



		// side image: align with front image
		cv::Mat transMat = cv::Mat::zeros(2, 3, CV_64F);
		transMat.at<double>(0, 0) = transMat.at<double>(1, 1) = 1.0;
		transMat.at<double>(1, 2) = m_FaceGeometry.getDetectedPoint(FaceGeometry::FrontLeftEye).y - m_FaceGeometry.getDetectedPoint(FaceGeometry::SideEye).y;
		
		// transform side image
		cv::warpAffine(m_Textures[sideImgNr], m_Textures[sideImgNr], transMat, m_Textures[frontImgNr].size());

		// transform all side coordinates
		m_FaceGeometry.transform(FaceGeometry::SideEye, transMat);
		m_FaceGeometry.transform(FaceGeometry::SideNoseTip, transMat);
		m_FaceGeometry.transform(FaceGeometry::SideChin, transMat);

		//dbgShow(m_Textures[sideImgNr], "createTextures: sideImage translated");		


		// cut out regions of interest, but do it in a way such that the y coordinate (position of eyes) still aligns between front and side image
		cv::Rect frontBoundingBox, sideBoundingBox;

		// front		
		frontBoundingBox.x = m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontLeftCheek).x;
		frontBoundingBox.y = m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontLeftEye).y;
		frontBoundingBox.width = m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontRightCheek).x - m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontLeftCheek).x;
		frontBoundingBox.height = m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideChin).y - m_FaceGeometry.getDetectedPointInt(FaceGeometry::FrontLeftEye).y;
		// add. texture
		frontBoundingBox.y = frontBoundingBox.y*(1 - m_AddTexture);
		frontBoundingBox.height = frontBoundingBox.height*(1 + 2 * m_AddTexture);

		// side
		sideBoundingBox.x = m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideBack).x;
		sideBoundingBox.y = m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideEye).y;
		sideBoundingBox.width = m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideNoseTip).x - m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideBack).x;
		sideBoundingBox.height = m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideChin).y - m_FaceGeometry.getDetectedPointInt(FaceGeometry::SideEye).y;
		// add. texture
		sideBoundingBox.y = frontBoundingBox.y*(1 - m_AddTexture/2);
		sideBoundingBox.height = frontBoundingBox.height*(1 +  m_AddTexture);


		m_Textures[frontImgNr](frontBoundingBox).copyTo(m_Textures[frontImgNr]);
		m_Textures[sideImgNr](sideBoundingBox).copyTo(m_Textures[sideImgNr]);
	
		// and now cut out the regions according to bounding box

		// resize to OpenGL compatible texture size (2^n x 2^n)
		const size_t texSize = 256;
		cv::resize(m_Textures[sideImgNr], m_Textures[sideImgNr], cv::Size(texSize, texSize));
		cv::resize(m_Textures[frontImgNr], m_Textures[frontImgNr], cv::Size(texSize, texSize));

		// calc the position of the eyes in the texture [0..1]
		// left eye
		cv::Point3d texLeftEye;
		texLeftEye.x = (m_FaceGeometry.getDetectedPoint(FaceGeometry::FrontLeftEye).x - frontBoundingBox.x) / frontBoundingBox.width;
		texLeftEye.y = (m_FaceGeometry.getDetectedPoint(FaceGeometry::FrontLeftEye).y - frontBoundingBox.y) / frontBoundingBox.height;
		m_FaceGeometry.setDetectedPoint(FaceGeometry::TextureLeftEye, texLeftEye);

		// right eye
		cv::Point3d texRightEye;
		texRightEye.x = (m_FaceGeometry.getDetectedPoint(FaceGeometry::FrontRightEye).x - frontBoundingBox.x) / frontBoundingBox.width;
		texRightEye.y = (m_FaceGeometry.getDetectedPoint(FaceGeometry::FrontRightEye).y - frontBoundingBox.y) / frontBoundingBox.height;
		m_FaceGeometry.setDetectedPoint(FaceGeometry::TextureRightEye,texRightEye);


		//dbgShow(m_Textures[frontImgNr], "createTextures: aligned and resized", frontImgNr); already shown in gui
		//dbgShow(m_Textures[sideImgNr], "createTextures: aligned and resized", sideImgNr);
	}



	cv::Rect Detection::getBoundingBox(const cv::Mat& color)
	{
		cv::Mat gray, bin;
		cv::cvtColor(color, gray, CV_BGR2GRAY);
		cv::threshold(gray, bin, 1, 255, cv::THRESH_BINARY);

		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(bin, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
		cv::Rect boundingBox = cv::boundingRect(contours[0]);

		return boundingBox;
	}



	cv::Mat Detection::combineVertically(const cv::Mat& a, const cv::Mat& b) const
	{
		assert(a.size()==b.size() && a.type()==b.type());

		cv::Mat canvas(a.rows * 2, a.cols, a.type());

		a.copyTo(canvas(cv::Rect(0, 0, a.cols, a.rows)));
		b.copyTo(canvas(cv::Rect(0, a.rows, a.cols, a.cols)));
	
		return canvas;
	}


	void onColorThresholdsTrackbar(int val, void* ptr)
	{
		Detection* detection = static_cast<Detection*>(ptr);
		detection->m_OffsetCR = val - 10;
		detection->m_OffsetCB = val - 10;
		detection->doFaceExtraction();

		cv::imshow("Select color threshold", detection->combineVertically(detection->m_FaceExtracted[detection->frontImgNr], detection->m_FaceExtracted[detection->sideImgNr]));

	}

	void Detection::doFaceExtractionGUI()
	{	
		// call it one time with default values
		doFaceExtraction();

		// show gui
		cv::namedWindow("Select color threshold");
		cv::createTrackbar("Threshold", "Select color threshold", &m_ColorThresValue, 20, onColorThresholdsTrackbar,this);
		cv::imshow("Select color threshold", combineVertically(m_FaceExtracted[frontImgNr],m_FaceExtracted[sideImgNr]));
		
		// show until any key			
		cv::waitKey();
		cv::destroyWindow("Select color threshold");
	}


	void onTextureAdjustmentTrackbar(int val, void* ptr)
	{
		Detection* detection = static_cast<Detection*>(ptr);
		detection->m_AddTexture = val/100.0;
		detection->m_FaceGeometry = detection->m_FaceGeometryBackup;
		detection->createTextures();

		cv::imshow("Resulting textures", detection->combineVertically(detection->m_Textures[detection->frontImgNr], detection->m_Textures[detection->sideImgNr]));
	}


	void Detection::createTexturesAndShowResultsGUI()
	{
		// 1. facial points
		// show gui
		cv::namedWindow("Resulting facial components");
		cv::imshow("Resulting facial components", m_FacialPointsGUI);

		// show until any key			
		cv::waitKey();
		cv::destroyWindow("Resulting facial components");

		// 2. textures
		// show gui
		m_FaceGeometryBackup = m_FaceGeometry;
		createTextures();
		cv::namedWindow("Resulting textures");
		cv::createTrackbar("adjust size", "Resulting textures", 0, 25, onTextureAdjustmentTrackbar, this);
		cv::imshow("Resulting textures", combineVertically(m_Textures[frontImgNr], m_Textures[sideImgNr]));

		// show until any key			
		cv::waitKey();
		cv::destroyWindow("Resulting textures");
	}

}