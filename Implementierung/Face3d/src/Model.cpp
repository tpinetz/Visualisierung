#include "Model.hpp"
#include "ShaderLoader.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Texture.hpp"


// Implementation follows: http://www.learnopengl.com/#!Model-Loading/Model
namespace Face3D
{
	// CTOR
	Model::Model(const ModelInfo& modelInfo)
	:m_ModelInfo(modelInfo)
	{
		m_FaceCoords.fromFile("ipc/faceGeometry.txt");
		load(modelInfo.modelPath);
		m_TextureFrontID = Texture::Instance().loadFromImage(modelInfo.textureFront);
		m_TextureSideID = Texture::Instance().loadFromImage(modelInfo.textureSide);
	}


	// load model if not yet cached
	void Model::load(const std::string& path)
	{
		m_pMeshes = std::make_shared<std::vector<Mesh>>();
		m_ShaderID = ShaderLoader::Instance().getProgram("Default");
		m_MVPMatrixLocation = glGetUniformLocation(m_ShaderID, "mvpMatrix");
		m_TextureFrontSamplerLocation = glGetUniformLocation(m_ShaderID, "textureFrontSampler");
		m_TextureSideSamplerLocation = glGetUniformLocation(m_ShaderID, "textureSideSampler");			
		m_ChinVerticalPosLocation = glGetUniformLocation(m_ShaderID, "chinVerticalPos");
		m_EyeVerticalPosLocation = glGetUniformLocation(m_ShaderID, "eyeVerticalPos");

		// Create an instance of the importer class
		Assimp::Importer importer;
		// Read in the given file into a scene and set some (example) postprocessing
		const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);// | aiProcess_FlipUVs);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::string error = importer.GetErrorString();
			throw std::exception("Failed to load model. ASSIMP-ERROR");
		}
		// Start processing nodes
		processNode(scene->mRootNode, scene);
	}

	void Model::processNode(aiNode *node, const aiScene *scene)
	{
		// Process all the node's meshes
		for (GLuint a = 0; a < node->mNumMeshes; a++)
		{
			aiMesh *currentMesh = scene->mMeshes[node->mMeshes[a]];
			m_pMeshes->push_back(processMesh(currentMesh, scene, std::string(node->mName.C_Str())));
		}
		// Do the same for all child-nodes
		for (GLuint a = 0; a < node->mNumChildren; a++)
		{
			processNode(node->mChildren[a], scene);
		}
	}	


	void Model::calcScalingFactors()
	{
		// dimension according to detection
		const glm::vec3 detectedFaceDimensions = m_FaceCoords.getPoint(FaceCoordinates3d::FaceDimensions);

		// dimensions according to generic model 
		const glm::vec3& modelDimensions = m_ModelInfo.modelDimension;

		// change mesh: global changes to mesh	
		// calc resizing factors
		m_fx = detectedFaceDimensions.x / modelDimensions.x;
		m_fy = -detectedFaceDimensions.y / modelDimensions.y; // we have to invert this coordinate, such that image and model coordinate system look into same direction
		m_fz = detectedFaceDimensions.z / modelDimensions.z;
	}


	glm::vec3 Model::moveGenericVertex(const glm::vec3& genericVertex)
	{
		glm::vec3 res = glm::vec3(genericVertex.x*m_fx, genericVertex.y*m_fy, genericVertex.z*m_fz);;

		
		// move mouth
		for (size_t i = 0; i < m_ModelInfo.allMouthVertices.size(); ++i)
		{
			if (isInsideEpsBall(genericVertex, m_ModelInfo.allMouthVertices[i]))
			{

				// position of mouth, relative to chin: calc this both in the model and in the image

				// 1. model
				glm::vec3 mouthInModel = m_ModelInfo.mouth - m_ModelInfo.chin;
				mouthInModel = glm::vec3(mouthInModel.x*m_fx, mouthInModel.y*m_fy, mouthInModel.z*m_fz);

				// 2. image
				glm::vec3 mouthInImage = m_FaceCoords.getPoint(FaceCoordinates3d::Mouth) - m_FaceCoords.getPoint(FaceCoordinates3d::Chin);

				// difference beween them
				glm::vec3 diffVec = mouthInImage - mouthInModel;

				// for the mouth, we're just interested in the vertical position
				diffVec.x = 0;
				diffVec.z = 0;

				res = res + diffVec;
			}		
		}

		// move nose
		for (size_t i = 0; i < m_ModelInfo.allNoseVertices.size(); ++i)
		{
			if (isInsideEpsBall(genericVertex, m_ModelInfo.allNoseVertices[i]))
			{

				// position of nose, relative to chin: calc this both in the model and in the image

				// 1. model
				glm::vec3 noseInModel = m_ModelInfo.nose - m_ModelInfo.chin;
				noseInModel = glm::vec3(noseInModel.x*m_fx, noseInModel.y*m_fy, noseInModel.z*m_fz);

				// 2. image
				glm::vec3 noseInImage = m_FaceCoords.getPoint(FaceCoordinates3d::Nose) - m_FaceCoords.getPoint(FaceCoordinates3d::Chin);

				// difference beween them
				glm::vec3 diffVec = noseInImage - noseInModel;

				// for the nose, we're just interested in the vertical position
				diffVec.x = 0;
				diffVec.z = 0;

				res = res + diffVec;
			}
		}

		
				
		return res;
	}


	Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene, std::string name)
	{		
		calcScalingFactors();

		// calc positions of important vertices in resized model
		glm::vec4 posLeftEye = glm::vec4(m_ModelInfo.leftEye,1.0f)*m_fx;

				
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		for (GLuint a = 0; a < mesh->mNumVertices; a++)
		{
			Vertex vertex;
			
			// Position
			glm::vec3 originalPos(mesh->mVertices[a].x, mesh->mVertices[a].y, mesh->mVertices[a].z);
			vertex.position = glm::vec4(moveGenericVertex(originalPos), 1.0f);

			// Normal		
			vertex.normal = glm::vec4(mesh->mNormals[a].x, mesh->mNormals[a].y, mesh->mNormals[a].z,1.0f);
			

			// save new vertex
			vertices.push_back(vertex);
		}

		// Collect all the indices from the faces of the mesh 
		for (GLuint a = 0; a < mesh->mNumFaces; a++)
		{
			aiFace face = mesh->mFaces[a];
			for (GLuint b = 0; b < face.mNumIndices; b++)
			{
				indices.push_back(face.mIndices[b]);
			}
		}



		return Mesh(vertices, indices);
	}

	void Model::render()
	{
		assert(!m_pMeshes->empty());

		// enable shader
		glUseProgram(m_ShaderID);
		glUniform1i(m_TextureFrontSamplerLocation, 0);
		glUniform1i(m_TextureSideSamplerLocation, 1);
		GLdouble chinYPos = m_ModelInfo.chin.y * m_fy;
		GLdouble eyeYPos = m_ModelInfo.leftEye.y * m_fy;
		glUniform1f(m_ChinVerticalPosLocation, chinYPos);
		glUniform1f(m_EyeVerticalPosLocation, eyeYPos);
		
		// calc MVP matrix			
		m_MVPMatrix = glm::rotate(glm::mat4(1.0f), m_RotationAngle, glm::vec3(0, 1, 0));
		m_MVPMatrix = glm::scale(m_MVPMatrix, glm::vec3(m_ScaleVal, -m_ScaleVal, m_ScaleVal)); // flip back y coordinate!
			
		// set MVP matrix
		glUniformMatrix4fv(m_MVPMatrixLocation, 1, GL_FALSE, &m_MVPMatrix[0][0]);

		// activate texture unit
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_TextureFrontID);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_TextureSideID);


		// render mesh
		for (size_t i = 0; i < (*m_pMeshes).size(); ++i)
		{
			(*m_pMeshes)[i].render();
		}	


		// disable shader
		glUseProgram(0);
	}


	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
	:m_Vertices(vertices)
	,m_Indices(indices)
	{			
		setup();
	}


	void Mesh::setup()
	{
		// Create VAO, VBO and ELEMENTBUFFER
		glGenVertexArrays(1, &m_VaoID);
		glGenBuffers(1, &m_VboID);
		glGenBuffers(1, &m_EboID);

		// Bind them all
		glBindVertexArray(m_VaoID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EboID);

		// Fill them with data
		glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(GLuint), &m_Indices[0], GL_STATIC_DRAW);

		// Set vertex attribute pointers
		// 0 = pos
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		// 1 = normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

		// Last but not least, unbind VAO		
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void Mesh::render()
	{
		// Retrieve saved data / Bind VAO
		glBindVertexArray(m_VaoID);

		// draw all triangles
		glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);

		// Unbind VAO
		glBindVertexArray(0);
	}
}