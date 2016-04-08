#include "GLDebug.hpp"
#include <stdio.h>

void APIENTRY debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	// add IDs of messages which you want to ignore 
	switch (id)
	{
	case 131169: // The driver allocated storage for renderbuffer
	case 131185: // glBufferData
	case 131076:
	case 131154:
		return;
	}

	printf("\n*********OpenGL Debug Begin****************\n");
	printf("ID: %d\n", id);
	printf("Message: %s\n", message);

	printf("Source: ");
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		printf("API");
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		printf("Window System");
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		printf("Shader Compiler");
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		printf("Third Party");
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		printf("Application");
		break;
	case GL_DEBUG_SOURCE_OTHER:
		printf("Other");
		break;
	}
	printf("\n");


	printf("Type: ");
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		printf("Error");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		printf("Deprecated Behaviour");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		printf("Undefined Behaviour");
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		printf("Portability");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		printf("Performance");
		break;
	case GL_DEBUG_TYPE_MARKER:
		printf("Marker");
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		printf("Push Group");
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		printf("Pop Group");
		break;
	case GL_DEBUG_TYPE_OTHER:
		printf("Other");
		break;
	}
	printf("\n");


	printf("Severity: ");
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		printf("High");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		printf("Medium");
		break;
	case GL_DEBUG_SEVERITY_LOW:
		printf("Low");
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		printf("Notification");
		break;
	}

	printf("\n*********OpenGL Debug End****************\n");
	printf("\n\n");
}