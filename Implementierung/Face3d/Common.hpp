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
	
}

