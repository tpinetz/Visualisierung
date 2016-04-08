#define STB_IMAGE_IMPLEMENTATION // Needed for stb_image to work! See sbt_image.h for more information!

#include "Texture.hpp"

namespace Face3D
{

	Texture::Texture()
		:m_CurrSetting(TextureLinear)
	{
		glGenSamplers(1, &samplerID);
	}

	Texture& Texture::Instance()
	{
		static Texture instance;
		return instance;
	}

	GLuint Texture::getSamplerID()
	{
		return samplerID;
	}

	GLuint Texture::loadFromImage(const std::string& fileName)
	{
		// already in cache?
		auto it = m_TextureCache.find(fileName);
		if (it != m_TextureCache.end())
		{
			return it->second;
		}

		int textureWidth = 0;
		int textureHeight = 0;
		int components = 0;

		unsigned char *image = stbi_load(fileName.c_str(), &textureWidth, &textureHeight, &components, STBI_rgb);

		if (image == 0)
		{
			throw std::exception("Could not load texture");
		}
			

		// Create an OpenGL texture
		GLuint textureID;
		glGenTextures(1, &textureID);
		// Bind it
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Hand the image to OpenGL
		const int variant = 1; // better use 1 as it works on any PC I tried, while 2 sometimes yields strange effects (black textures)
		switch (variant)
		{
		case 1:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			break;

		case 2:
			glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGB8, textureWidth, textureHeight);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGB, GL_UNSIGNED_BYTE, image);
			break;
		}
			

		glGenerateMipmap(GL_TEXTURE_2D);

		// Set parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Free image / memory
		stbi_image_free(image);
		// Add textureID to cache
		m_TextureCache[fileName] = textureID;
		// unbind
		glBindTexture(GL_TEXTURE_2D, 0);

		return textureID;
	}

	void Texture::changeTextureSettings(TextureSetting setting)
	{
		// already set - nothing to do
		if (m_CurrSetting == setting)
		{
			return;
		}

		m_CurrSetting = setting;

		// go over all textures and change setting
		for (auto it = m_TextureCache.begin(); it != m_TextureCache.end(); ++it)
		{
			const GLuint texID = it->second;

			glBindTexture(GL_TEXTURE_2D, texID);

			switch (setting)
			{
				case TextureLinear:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					break;

				case TextureNearest:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					break;

				case TextureMIPNearest:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					break;

				case TextureMIPLinear:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					break;
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	
}