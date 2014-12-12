// Parts of this code are taken from http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
#ifndef MODEL_H
#define	MODEL_H


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "vertex.h"
#include <FreeImagePlus.h>
#include "mesh.h"
#include "lightingEngine.h"

using namespace std;

class Model {

public:
	Model();

	~Model();

	bool LoadMesh(const std::string& Filename);

	void Render(glm::mat4 mvp, glm::mat4 model, LightingEngine lightingEngine);

private:
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);
	void InitMesh(const aiScene* pScene, unsigned int Index, aiColor3D color);
	void InitMaterials(const aiScene* pScene, unsigned index);
	void Init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices);
	void Clear();

	GLuint VB;
	GLuint IB;
	unsigned int NumIndices;
	unsigned int MaterialIndex;

	std::vector<Mesh> m_Entries;
};

#endif