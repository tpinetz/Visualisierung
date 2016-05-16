#pragma once

// Common
#include <vector>
#include <string>
#include <memory>
#include <map>
// Assimp
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
// Helpers
#include "FaceCoordinates3d.hpp"
#include "GLHeader.hpp"



namespace Face3D
{
	// single vertex
	struct Vertex
	{
		glm::vec4 position;
		glm::vec4 normal;
	};

	// triangle mesh
	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, glm::vec3 positionInModel);
		void render();		

	private:
		std::vector<Vertex> m_Vertices;
		glm::vec3 m_MinVertex;
		glm::vec3 m_MaxVertex;
		glm::vec3 m_MeanVertex;
		std::vector<GLuint> m_Indices;
		GLuint m_VaoID=0, m_VboID=0, m_EboID=0;	
		glm::vec3 m_positionInModel;
		
		void setup();
		void calcMeshInfo();
		void moveVertices();
	};

	// model of an object containing meshes and vertices
	class Model
	{
		public:
			struct ModelInfo
			{
				std::string modelPath;
				std::string textureFront;
				std::string textureSide;

				glm::vec3 modelDimension;
				glm::vec3 leftEye, rightEye, mouth, chin;
			};

			Model(const ModelInfo& modelInfo);
			void rotate(GLfloat val){ m_RotationAngle = val; }
			void scale(GLfloat val){ m_ScaleVal = val; }
			void render();

		private:				
			ModelInfo m_ModelInfo;
			FaceCoordinates3d m_FaceCoords;
			
			GLuint m_TextureFrontID = 0;
			GLuint m_TextureSideID = 0;
			GLuint m_SamplerID = 0;
			std::shared_ptr<std::vector<Mesh>> m_pMeshes;
			GLuint m_ShaderID=0;
			GLuint m_MVPMatrixLocation = 0;
			GLuint m_TextureFrontSamplerLocation = 0;
			GLuint m_TextureSideSamplerLocation = 0;
			GLuint m_ChinVerticalPosLocation = 0;
			GLuint m_EyeVerticalPosLocation = 0;
			glm::mat4 m_MVPMatrix;
			GLfloat m_RotationAngle = 0.0f;
			GLfloat m_ScaleVal = 1.0f;
			std::string m_LEyeName = "EyeL";
			std::string m_REyeName = "EyeR";
			std::string m_MouthName = "Mouth";
			std::string m_NoseName = "Nose";
			std::string m_ChinName = "Chin";
			GLdouble m_fx=0, m_fy=0, m_fz=0;

			// load mesh data from file
			void load(const std::string& path);
			void processNode(aiNode *node, const aiScene *scene);
			Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string name);
		};	
}
