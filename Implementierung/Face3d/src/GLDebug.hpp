#pragma once

#include "GLHeader.hpp"

/** callback for OpenGL to show debug infos/errors */
void APIENTRY debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);