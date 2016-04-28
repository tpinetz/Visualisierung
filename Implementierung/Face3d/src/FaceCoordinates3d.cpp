#include "FaceCoordinates3d.hpp"

namespace Face3D
{

	glm::vec3 FaceCoordinates3d::getPoint(FacialPoints3d type) const
	{
		return m_Points[type];
	}

	
	void FaceCoordinates3d::fromFile(const std::string& fn)
	{
		std::ifstream f(fn.c_str());

		// order (must be the same in both programs!): leftEye, rightEye, nose, mouth
		m_Points[LeftEye] = fileToPoint(f);
		m_Points[RightEye] = fileToPoint(f);
		m_Points[Nose] = fileToPoint(f);
		m_Points[Mouth] = fileToPoint(f);
	}


	glm::vec3 FaceCoordinates3d::fileToPoint(std::ifstream& f)
	{
		glm::vec3 p;

		f >> p.x;
		f >> p.y;
		f >> p.z;

		return p; 
	}
}