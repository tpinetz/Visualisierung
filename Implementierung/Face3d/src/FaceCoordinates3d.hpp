#pragma once

#include "GLHeader.hpp"
#include <fstream>


namespace Face3D
{
	class FaceCoordinates3d
	{
	public:
		/** the points which we've extracted so far */
		enum FacialPoints3d{ LeftEye, RightEye, Mouth, Nose, Chin, FaceDimensions, TextureLeftEye, TextureRightEye, InvalidPoint };

		/** specify the point (e.g. nose) and get 3d coordinates of it */
		glm::vec3 getPoint(FacialPoints3d type) const;

		/** ipc - deserialize data */
		void fromFile(const std::string& fn);

	private:
		/** 3d coordinates of facial point (e.g. nose) */
		glm::vec3 m_Points[InvalidPoint];

		/** load a single point from a file */
		glm::vec3 fileToPoint(std::ifstream& f);
	};
}