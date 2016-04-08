#include "GLDebug.hpp"
#include <iostream>

void APIENTRY debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	std::cout<<"\n*********OpenGL Debug Begin****************\n";
	std::cout << "ID: " << id << "\n";
	std::cout << "Message: " << message << "\n";

	std::cout<<"Source: ";
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		std::cout << "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		std::cout << "Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		std::cout << "Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		std::cout << "Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		std::cout << "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		std::cout << "Other";
		break;
	}
	std::cout << "\n";


	std::cout << "Type: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "Deprecated Behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "Undefined Behaviour";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		std::cout << "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		std::cout << "Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		std::cout << "Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cout << "Other";
		break;
	}
	std::cout << "\n";


	std::cout << "Severity: ";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "High";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "Medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "Low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cout << "Notification";
		break;
	}

	std::cout << ("\n*********OpenGL Debug End****************\n\n\n");
}
