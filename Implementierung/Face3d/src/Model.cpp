#include "Model.hpp"
#include "ShaderLoader.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Texture.hpp"

// Implementation follows: http://www.learnopengl.com/#!Model-Loading/Model
namespace Face3D
{
	// CTOR
	Model::Model(const std::string& modelPath, const std::string& textureFront, const std::string& textureSide)
	{
		load(modelPath);
		m_TextureFrontID = Texture::Instance().loadFromImage(textureFront);
		m_TextureSideID = Texture::Instance().loadFromImage(textureSide);
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
			m_pMeshes->push_back(processMesh(currentMesh, scene));
		}
		// Do the same for all child-nodes
		for (GLuint a = 0; a < node->mNumChildren; a++)
		{
			processNode(node->mChildren[a], scene);
		}
	}

	Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		//std::vector<Textures::Texture> textures;

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

			// Texture coordinates
			// Check if we actually have texture coordinates for the current mesh
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vector;
				// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vector.x = mesh->mTextureCoords[0][a].x;
				vector.y = mesh->mTextureCoords[0][a].y;
				vertex.textureCoordinates = vector;
			}
			else
			{
				vertex.textureCoordinates = glm::vec2(0.0f, 0.0f);
			}

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
		// enable shader
		glUseProgram(m_ShaderID);
		glUniform1i(m_TextureFrontSamplerLocation, 0);
		glUniform1i(m_TextureSideSamplerLocation, 1);

		for (GLuint a = 0; a < m_pMeshes->size(); a++)
		{
			// calc MVP matrix
			GLfloat rot = std::fmod(glfwGetTime(), 2 * M_PI);
			m_MVPMatrix = glm::rotate(glm::mat4(1.0f), rot, glm::vec3(0, 1, 0));
			m_MVPMatrix = glm::scale(m_MVPMatrix, glm::vec3(0.25));
			//m_MVPMatrix = glm::translate(m_MVPMatrix, -(*m_pMeshes)[a].getCenter());
			
			// set MVP matrix
			glUniformMatrix4fv(m_MVPMatrixLocation, 1, GL_FALSE, &m_MVPMatrix[0][0]);

			// activate texture unit
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_TextureFrontID);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_TextureSideID);


			// render mesh
			(*m_pMeshes)[a].render();
		}

		// disable shader
		glUseProgram(0);
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
	:m_Vertices(vertices)
	, m_Indices(indices)
	{	
		setup();
		calcMeshInfo();
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
		// 2 = texture coordinates
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, textureCoordinates));

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