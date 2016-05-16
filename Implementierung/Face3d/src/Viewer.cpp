#include "Viewer.hpp"
#include <exception>
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
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		// setup glew
		if (glewInit() != GLEW_OK)
		{
			throw std::exception("glewInit() != GLEW_OK");
		}


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
		

		// enable depth test and back face culling
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);		

		//#define DEBUG_DRAW_LINES
#ifdef DEBUG_DRAW_LINES
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
	}


	// load coordinates of important vertices in generic model (this should be loaded from a file, e.g. CSV or XML)
	void Viewer::loadModelCoordinates(Model::ModelInfo& modelInfo)
	{
		// dimensions of the model
		modelInfo.modelDimension = glm::vec3(3.4, 2.0, 2.7);		

		// -- center vertices of components --
		modelInfo.leftEye = glm::vec3(-0.2, 0.65, 0.78);
		modelInfo.rightEye = glm::vec3(-0.2, -0.65, 0.78);
		modelInfo.mouth = glm::vec3(-0.9, -0.62, 0);
		modelInfo.nose = glm::vec3(-1.368, 0.0249, 0);
		modelInfo.chin = glm::vec3(-0.87, -1.36, 0);

		// -- all vertices of components --

		// mouth
		modelInfo.allMouthVertices =
		{
			// middle line
			glm::vec3(-0.9, -0.62, 0), glm::vec3(-0.919, -0.627, 0.21), glm::vec3(-0.919, -0.627, -0.21), glm::vec3(-0.66, -0.731, 0.506), glm::vec3(-0.66, -0.731, -0.506),

			// upper line
			glm::vec3(-1, -0.495, 0), glm::vec3(-1.02, -0.44, 0.2), glm::vec3(-1.02, -0.44, -0.2),

			// lower middle line
			glm::vec3(-0.998, -0.772, 0), glm::vec3(-0.998, -0.772, 0.186), glm::vec3(-0.998, -0.772, -0.186),

			// lower line
			glm::vec3(-0.842, -0.951, 0.169), glm::vec3(-0.842, -0.951, -0.169), glm::vec3(-0.842, -0.951, 0)
		};

		// nose
		modelInfo.allNoseVertices =
		{
			glm::vec3(-1.368, 0.0249, 0), glm::vec3(-1.265, 0.0248, 0.177), glm::vec3(-1.265, 0.0248, -0.177)
		};

		// left eye
		modelInfo.allLeftEyeVertices = { glm::vec3(-0.2, 0.65, 0.78) };

		// right eye
		modelInfo.allLeftEyeVertices = { glm::vec3(-0.2, -0.65, 0.78) };
	}


	void Viewer::run()
	{

		// load model, front and side texture
		Model::ModelInfo modelInfo;
		// file path
		modelInfo.modelPath = "models/simpleSingleMesh.obj";
		modelInfo.textureFront = "ipc/front.jpg";
		modelInfo.textureSide = "ipc/side.jpg";
		
		loadModelCoordinates(modelInfo);

		// load model
		Model model(modelInfo);
		
		// transformation for model viewing
		GLfloat rotationsVal = 0.0f;
		GLfloat scaleVal = 0.002f;
		const GLfloat rotValIncrease = 0.5f;
		const GLfloat scaleValIncrease = 0.05f;

		// initial settings
		model.rotate(rotationsVal);
		model.scale(scaleVal);
		GLfloat oldTime = glfwGetTime();

		while (!glfwWindowShouldClose(m_pWindow))
		{
			GLfloat newTime = glfwGetTime();
			GLfloat deltaTime =  newTime - oldTime;
			oldTime = newTime;
			// clear window content
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(.5, .5, .5, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// draw
			model.render();

			// Swap buffers
			glfwSwapBuffers(m_pWindow);

			// key events: ESC, left, right
			glfwPollEvents();
			if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(m_pWindow, GL_TRUE);
			}

			if (glfwGetKey(m_pWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				rotationsVal += rotValIncrease * deltaTime;
				model.rotate(rotationsVal);
			}

			if (glfwGetKey(m_pWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				rotationsVal -= rotValIncrease * deltaTime;
				model.rotate(rotationsVal);
			}

			if (glfwGetKey(m_pWindow, GLFW_KEY_UP) == GLFW_PRESS)
			{
				scaleVal += scaleValIncrease * deltaTime;
				model.scale(scaleVal);
			}

			if (glfwGetKey(m_pWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				scaleVal -= scaleValIncrease * deltaTime;
				if (scaleVal < 0.0)
				{
					scaleVal = 0.0;
				}

				model.scale(scaleVal);
			}
		}
	}


}