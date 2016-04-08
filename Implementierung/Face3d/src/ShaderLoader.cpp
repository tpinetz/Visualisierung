#include "ShaderLoader.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>



namespace Face3D
{
	ShaderLoader& ShaderLoader::Instance()
	{
		static ShaderLoader instance;
		return instance;
	}

	// get shader program
	GLuint ShaderLoader::getProgram(const std::string& shaderClassName)
	{
		auto it = m_ShaderProgramMap.find(shaderClassName);

		// not yet compiled - compile it
		if (it == m_ShaderProgramMap.end())
		{
			GLuint id = loadVertexAndFragmentShader(shaderClassName);
			m_ShaderProgramMap[shaderClassName] = id;
			return id;
		}
		// already compiled - return ID
		else
		{
			return it->second;
		}
	}

	void ShaderLoader::addToCache(const GLuint programID, const std::string &shaderProgramName)
	{
		auto it = m_ShaderProgramMap.find(shaderProgramName);
		if (it == m_ShaderProgramMap.end())
		{
			m_ShaderProgramMap[shaderProgramName] = programID;
		}
	}

	GLuint ShaderLoader::loadShader(const std::string& shaderClassName, GLenum shaderType)
	{
		const std::string shaderDir("shader\\");
		const std::string pathToShader = shaderDir + shaderClassName;

		GLuint shaderID = glCreateShader(shaderType);

		std::string shaderCode = readInShaderCode(pathToShader);

		if (shaderCode.empty())
			throw std::exception("Could not open file");

		GLint result = GL_FALSE;

		std::cout << "Compiling shader: " << pathToShader << "\n";
		compileShader(shaderID, shaderCode);
		checkShader(shaderID, result);

		return shaderID;
	}

	void ShaderLoader::deleteShader(const GLuint shaderID)
	{
		glDeleteShader(shaderID);
	}

	GLuint ShaderLoader::createProgram()
	{
		return glCreateProgram();
	}

	void ShaderLoader::attachShader(const GLuint programID, const GLuint shaderID)
	{
		glAttachShader(programID, shaderID);
	}

	void ShaderLoader::linkProgram(const GLuint programID)
	{
		std::cout << "Linking program: " << programID << "\n";
		glLinkProgram(programID);
	}

	void ShaderLoader::checkProgram(const GLuint programID)
	{
		GLint result = GL_FALSE;
		int infoLogLength;

		glGetProgramiv(programID, GL_LINK_STATUS, &result);
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (result != GL_TRUE)
		{
			std::vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programID, infoLogLength, NULL, &ProgramErrorMessage[0]);

			// and throw error msg
			throw std::exception("Shader Linking Error");
		}
	}
	
	GLuint ShaderLoader::loadVertexAndFragmentShader(const std::string& shaderClassName)
	{
		const std::string pathToVertexShader = shaderClassName + ".vertexShader.txt";
		const std::string pathToFragmentShader = shaderClassName + ".fragmentShader.txt";

		GLuint vertexShaderID = loadShader(pathToVertexShader, GL_VERTEX_SHADER);
		GLuint fragmentShaderID = loadShader(pathToFragmentShader, GL_FRAGMENT_SHADER);

		// Create a program
		GLuint programID = createProgram();
		// Attach shaders
		attachShader(programID, vertexShaderID);
		attachShader(programID, fragmentShaderID);
		// Link program
		linkProgram(programID);
		// Check program
		checkProgram(programID);
		// Delete the shaders
		deleteShader(vertexShaderID);
		deleteShader(fragmentShaderID);

		return programID;
	}

	void ShaderLoader::compileShader(GLuint shaderID, const std::string &shaderCode)
	{
		char const *shaderSourcePointer = shaderCode.c_str();
		glShaderSource(shaderID, 1, &shaderSourcePointer, NULL);
		glCompileShader(shaderID);
	}

	void ShaderLoader::checkShader(GLuint shaderID, GLint result)
	{
		int infoLogLength;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (result != GL_TRUE)
		{
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderID, infoLogLength, NULL, &shaderErrorMessage[0]);

			throw std::exception("result != GL_TRUE");
		}
	}

	std::string ShaderLoader::readInShaderCode(const std::string& pathToShaderFile)
	{
		std::string shaderCode;
		std::ifstream shaderStream(pathToShaderFile.c_str(), std::ios::in);

		if (shaderStream.is_open())
		{
			std::string line = "";
			while (getline(shaderStream, line))
				shaderCode += "\n" + line;
			shaderStream.close();
		}

		return shaderCode;
	}
}