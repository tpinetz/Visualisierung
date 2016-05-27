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
		m_Points[Chin] = fileToPoint(f);
		m_Points[FaceDimensions] = fileToPoint(f);
		m_Points[TextureLeftEye] = fileToPoint(f);
		m_Points[TextureRightEye] = fileToPoint(f);
		m_Points[TextureChin] = fileToPoint(f);
		m_Points[TextureChin].y = glm::clamp(m_Points[TextureChin].y, 0.0f, 0.10f); // Our model cant rly be extended at the chin.
	}


	glm::vec3 FaceCoordinates3d::fileToPoint(std::ifstream& f)
	{
		/*
		coordinates are different between detection and modelling
		detection -> modelling:
		x -> z
		y -> y
		z -> x

		*/

		glm::vec3 p;

		f >> p.z;
		f >> p.y;
		f >> p.x;

		return p; 
	}
}