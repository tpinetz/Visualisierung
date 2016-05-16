#pragma once

#include "GLHeader.hpp"
#include "Model.hpp"


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

		// load coordinates of important vertices in generic model (this should be loaded from a file, e.g. CSV or XML)
		void loadModelCoordinates(Model::ModelInfo& modelInfo);
	};

}