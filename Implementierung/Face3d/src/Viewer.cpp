#include "Viewer.hpp"
#include <exception>
#include "Model.hpp"
#include "GLDebug.hpp"
#include <iostream>

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
		m_pWindow = glfwCreateWindow(m_WindowHeight, m_WindowHeight, "Face3d: Viewer. Press LEFT / RIGHT arrow key to rotate model.", 0, 0);

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


#define GL_DEBUG
#ifdef GL_DEBUG
		GLint flags = 0;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			std::cout << "Debug Output available!\n";
		}

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debugMessage, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
#endif
		


		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void Viewer::run()
	{

		// TODO: init world
		Model model("models/simple.obj", "ipc/front.jpg", "ipc/side.jpg");
		//Model model("models/complex.obj");
		
		GLfloat rotationsVal = 0.0f;
		const GLfloat rotationsValIncrease = 0.001f;
		while (!glfwWindowShouldClose(m_pWindow))
		{
			// clear window content
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// draw
			// TODO ...
			model.render();

			// Swap buffers
			glfwSwapBuffers(m_pWindow);

			// ESC
			glfwPollEvents();
			if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(m_pWindow, GL_TRUE);
			}

			if (glfwGetKey(m_pWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				rotationsVal += rotationsValIncrease;
				model.rotate(rotationsVal);
			}

			if (glfwGetKey(m_pWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				rotationsVal -= rotationsValIncrease;
				model.rotate(rotationsVal);
			}
		}
	}


}