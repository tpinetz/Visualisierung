#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <sstream>

namespace Face3D
{
	/** \brief  show intermediate results while debugging - does not do anything when compiled in release mode
	* \param img image to show
	* \param msg text message to be displayed in the window title
	* \param id optional id if the same title is used twice 
	*/
	inline void dbgShow(const cv::Mat img, const std::string& msg="", int id=0)
	{
#ifndef _DEBUG
		return;
#else
		static int ctr = 0;
		++ctr;

		std::stringstream ss;
		ss << "Face3d: " << msg << "("<<id<<"), #window: "<<ctr;
		cv::imshow(ss.str(), img);
		cv::waitKey(0);

//#define DUMP_TO_FILE
#ifdef DUMP_TO_FILE
		std::stringstream fn;
		fn << "out/" << ctr << ".png";
		cv::imwrite(fn.str(),img);
#endif
#endif
	}
}

