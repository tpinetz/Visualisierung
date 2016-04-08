#pragma once

#include "GLHeader.hpp"

namespace Face3D
{

	class Viewer
	{
	public:
		void initOpenGL();
		void run();


	private:
		GLFWwindow* m_pWindow = 0;
		const int m_WindowWidth = 640, m_WindowHeight = 640;
	};

}