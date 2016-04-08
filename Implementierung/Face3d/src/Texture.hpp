#ifndef MISHUNGERSTANDING_TEXTURES_TEXTURE
#define MISHUNGERSTANDING_TEXTURES_TEXTURE

#include <stb_image.h>
#include <map>
#include <string>
#include "GLHeader.hpp"

namespace Face3D
{
	
	class Texture
	{
		public:
			static Texture& Instance();
			GLuint Texture::loadFromImage(const std::string& fileName);
			GLuint getSamplerID();

			enum TextureSetting { TextureLinear, TextureNearest, TextureMIPNearest, TextureMIPLinear };
			TextureSetting getSetting() const { return m_CurrSetting; }
			void changeTextureSettings(TextureSetting setting);

		private:
			Texture();
			std::map<std::string, GLuint> m_TextureCache;
			GLuint samplerID = 0;
			TextureSetting m_CurrSetting;
	};

}

#endif
