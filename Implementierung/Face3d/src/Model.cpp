#include "Model.hpp"
#include "ShaderLoader.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Texture.hpp"
#include "FaceCoordinates3d.hpp"


// Implementation follows: http://www.learnopengl.com/#!Model-Loading/Model
namespace Face3D
{
	// CTOR
	Model::Model(const ModelInfo& modelInfo)
	:m_ModelInfo(modelInfo)
	{
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

	Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene, std::string name)
	{

		FaceCoordinates3d faceCoords; // <--- HIER SIND DIE 3D KOORDINATEN DRINNEN AUS DEN BILDERN, Zugriff auf linkes Auge z.B.: faceCoords.getPoint(FaceCoordinates3d::LeftEye)
		faceCoords.fromFile("ipc/faceGeometry.txt");
		
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		for (GLuint a = 0; a < mesh->mNumVertices; a++)
		{
			Vertex vertex;
			glm::vec4 vector;

			// Position
			vector.x = mesh->mVertices[a].x;
			vector.y = mesh->mVertices[a].y;
			vector.z = mesh->mVertices[a].z;
			vector.w = 1.0f;
			vertex.position = vector;

			// Normal
			vector.x = mesh->mNormals[a].x;
			vector.y = mesh->mNormals[a].y;
			vector.z = mesh->mNormals[a].z;
			vector.w = 0.0f;
			vertex.normal = vector;			

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

		glm::vec3 positionInModel = glm::vec3(0.0f, 0.0f, 0.0f);


		if (name.compare(0, m_LEyeName.size(), m_LEyeName) == 0) {
			positionInModel = faceCoords.getPoint(FaceCoordinates3d::LeftEye);
		}
		else if (name.compare(0, m_REyeName.size(), m_REyeName) == 0) {
			positionInModel = faceCoords.getPoint(FaceCoordinates3d::RightEye);
		}
		else if (name.compare(0, m_MouthName.size(), m_MouthName) == 0) {
			positionInModel = faceCoords.getPoint(FaceCoordinates3d::Mouth);
		}
		else if (name.compare(0, m_NoseName.size(), m_NoseName) == 0) {
			positionInModel = faceCoords.getPoint(FaceCoordinates3d::Nose);
		}
		else if (name.compare(0, m_ChinName.size(), m_ChinName) == 0) {
			positionInModel = faceCoords.getPoint(FaceCoordinates3d::Chin);
		}


		return Mesh(vertices, indices, positionInModel);
	}

	void Model::render()
	{
		assert(!m_pMeshes->empty());

		// enable shader
		glUseProgram(m_ShaderID);
		glUniform1i(m_TextureFrontSamplerLocation, 0);
		glUniform1i(m_TextureSideSamplerLocation, 1);
		
		// calc MVP matrix			
		m_MVPMatrix = glm::rotate(glm::mat4(1.0f), m_RotationAngle, glm::vec3(0, 1, 0));
		m_MVPMatrix = glm::scale(m_MVPMatrix, glm::vec3(m_ScaleVal));
			
		// set MVP matrix
		glUniformMatrix4fv(m_MVPMatrixLocation, 1, GL_FALSE, &m_MVPMatrix[0][0]);

		// activate texture unit
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_TextureFrontID);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_TextureSideID);


		// render mesh
		(*m_pMeshes)[0].render();
		(*m_pMeshes)[1].render();
		(*m_pMeshes)[2].render();
		(*m_pMeshes)[3].render();
		(*m_pMeshes)[4].render();
		

		// disable shader
		glUseProgram(0);
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, glm::vec3 positionInModel)
	:m_Vertices(vertices)
	, m_Indices(indices)
	, m_positionInModel(positionInModel)
	{			
		calcMeshInfo();
		moveVertices();
		setup();
	}

	void Mesh::moveVertices() {
		glm::vec3 adjustmentFactor = this->m_MeanVertex - this->m_positionInModel;

		for (size_t i = 0; i < m_Vertices.size();++i)
		{
			m_Vertices[i].position.x += adjustmentFactor.x;
			m_Vertices[i].position.y += adjustmentFactor.y;
			m_Vertices[i].position.z += adjustmentFactor.z;
		}
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

	void Mesh::calcMeshInfo()
	{
		assert(m_Vertices.size()>0);

		glm::vec4 meanVec;
		for (size_t i = 0; i < m_Vertices.size(); ++i)
		{
			glm::vec4 currPos = m_Vertices[i].position;


			if (m_MinVertex.x > currPos.x)
			{
				m_MinVertex.x = currPos.x;
			}
			if (m_MinVertex.y > currPos.y)
			{
				m_MinVertex.y = currPos.y;
			}
			if (m_MinVertex.z > currPos.z)
			{
				m_MinVertex.z = currPos.z;
			}


			if (m_MaxVertex.x < currPos.x)
			{
				m_MaxVertex.x = currPos.x;
			}
			if (m_MaxVertex.y < currPos.y)
			{
				m_MaxVertex.y = currPos.y;
			}
			if (m_MaxVertex.z < currPos.z)
			{
				m_MaxVertex.z = currPos.z;
			}


			meanVec+=m_Vertices[i].position;
		}
		
		meanVec /= m_Vertices.size();		
		m_MeanVertex = glm::vec3(meanVec);
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