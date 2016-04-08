#pragma once

// Common
#include <string>
#include <map>
// Helpers
#include "GLHeader.hpp"

namespace Face3D
{	
	class ShaderLoader
	{
		public:
			// get instance
			static ShaderLoader& Instance();

			// get shader program
			GLuint getProgram(const std::string& shaderClassName);

			GLuint loadShader(const std::string& shaderClassName, GLenum shaderType);
			void deleteShader(const GLuint shaderID);

			GLuint createProgram();
			void linkProgram(const GLuint programID);
			void attachShader(const GLuint programID, const GLuint shaderID);
			void checkProgram(const GLuint programID);

			void addToCache(const GLuint programID, const std::string &shaderProgramName);

		private:
			ShaderLoader(){}
			GLuint loadVertexAndFragmentShader(const std::string& shaderClassName);
			std::string readInShaderCode(const std::string& pathToShaderFile);

			void compileShader(GLuint shaderID, const std::string &shaderCode);
			void checkShader(GLuint shaderID, GLint result);
			
			std::map<std::string, GLuint> m_ShaderProgramMap;
	};
}
