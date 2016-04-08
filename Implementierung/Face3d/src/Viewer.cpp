#include "Viewer.hpp"
#include <exception>

namespace Face3D
{

	void Viewer::initOpenGL()
	{
		// init glfw
		if (!glfwInit())
		{
			throw "";
		}

		// setup window
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		m_pWindow = glfwCreateWindow(m_WindowHeight, m_WindowHeight, "Face3d: Viewer", 0, 0);

		// error?
		if (!m_pWindow)
		{
			glfwTerminate();
			throw std::exception("m_pWindow=0");
		}

		glfwMakeContextCurrent(m_pWindow);
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// setup glew
		if (glewInit() != GLEW_OK)
		{
			throw std::exception("glewInit() != GLEW_OK");
		}

		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
	}

	void Viewer::run()
	{
		while (!glfwWindowShouldClose(m_pWindow))
		{
			// clear window content
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(1, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// draw
			// TODO ...

			// Swap buffers
			glfwSwapBuffers(m_pWindow);

			// ESC
			glfwPollEvents();
			if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(m_pWindow, GL_TRUE);
			}
		}
	}


}