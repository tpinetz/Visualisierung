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
	/** is val around x, that means inside the intervall [x-eps, x+eps] */
	template<class T>
	bool isInsideEpsBall(const T& a, const T& b)
	{
		return glm::distance(a, b) < 0.01;
	}


	/** definition of a single vertex */
	struct Vertex
	{
		glm::vec4 position;
		glm::vec4 normal;
	};

	/** definition of a triangle mesh */
	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		void render();		

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<GLuint> m_Indices;
		GLuint m_VaoID=0, m_VboID=0, m_EboID=0;	
		
		void setup();
	};

	/** definition of a model of an object containing meshes and vertices */
	class Model
	{
		public:

			/** all informations we need to load the model and move the vertices around are stored in this structure */
			struct ModelInfo
			{
				std::string modelPath;
				std::string textureFront;
				std::string textureSide;

				glm::vec3 modelDimension;
				 
				/// center of face components, needed as reference points when moving around the other vertices
				glm::vec3 leftEye, rightEye, mouth, nose, chin;

				/// specify all vertices of a component which should be moved around
				std::vector<glm::vec3> allMouthVertices, allNoseVertices, allLeftEyeVertices, allRightEyeVertices;

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
			GLuint m_LEyeTexVerticalPosLocation = 0;
			GLuint m_REyeTexVerticalPosLocation = 0;
			GLuint m_ChinTexVerticalPosLocation = 0;
			glm::mat4 m_MVPMatrix;
			GLfloat m_RotationAngle = 0.0f;
			GLfloat m_ScaleVal = 1.0f;
			GLfloat m_fx=0, m_fy=0, m_fz=0;

			/// load mesh data from file
			void load(const std::string& path);
			void processNode(aiNode *node, const aiScene *scene);
			Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string name);

			/** calculate the scaling factors to resize the generic face such that it looks like the face on the images */
			void calcScalingFactors();

			/**  move a vrtex of the generic model to its final position according to the face detection */
			glm::vec3 moveGenericVertex(const glm::vec3& genericVertex);
		};	
}
