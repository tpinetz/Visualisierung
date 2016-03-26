#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace Face3D
{
	inline void dbgShow(const cv::Mat img,const std::string& msg="")
	{
		cv::imshow(msg, img);
		cv::waitKey(0);
	}

	inline void dbgContourToImg(std::vector<cv::Point> contour, cv::Mat& result) {
		for (auto point : contour) {
			std::cout << point << std::endl;
			result.at<int>(point) = 255;
		}
	}
	
}

