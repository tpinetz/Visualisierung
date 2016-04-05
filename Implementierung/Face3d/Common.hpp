#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <sstream>

namespace Face3D
{

	inline void dbgShow(const cv::Mat img, const std::string& msg="", int id=0)
	{
#ifndef _DEBUG
		return;
#else
		std::stringstream ss;
		ss << "Face3d: " << msg << "("<<id<<")";
		cv::imshow(ss.str(), img);
		cv::waitKey(0);
#endif
	}

	inline void dbgContourToImg(std::vector<cv::Point> contour, cv::Mat& result) {
		for (auto point : contour) {
			std::cout << point << std::endl;
			result.at<int>(point) = 255;
		}
	}
	
}

