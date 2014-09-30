#ifndef LOADER_H
#define LOADER_H

#include <assimp/Importer.hpp> //includes the importer, which is used to read our obj file
#include <assimp/scene.h> //includes the aiScene object
#include <assimp/postprocess.h> //includes the postprocessing variables for the importer
#include <assimp/color4.h> //includes the aiColor4 object, which is used to handle the colors from the mesh objects

struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};


bool loadMesh(const std::string &fileName){
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(fileName.c_str() ,aiProcess_Triangulate);

  if(scene){
    return InitFromScene(scene, fileName);
  }
  else{
    printf("Error parsing '%s': '%s'\n", fileName.c_str(), importer.GetErrorString());
  }
  return false;
}

bool InitFromScene(const aiscene *scene, const std::string& fileName){
  m_Entries.resize(scene->mNumMeshes);
  m_Textures.resize(scene->mNumMaterials);

  //initialize the meshes in scene
  for(unsigned int i=0; i<m_entries.size(); i++){
    const aiMesh* aiMesh = scene->mMeshes[i];
    InitMesh(i, aiMesh);
  }
  return InitMaterials(scene, fileName);
}

aiMesh *mesh = scene->mMeshes[0];

Vertex *vertexArray;
Vertex *normalArray;
Vertex *uvArray;

int numVerts;

numVerts = mesh->mNumFaces*3;

vertexArray = new Vertex[mesh->mNumFaces*3*3];
normalArray = new Vertex[mesh->mNumFaces*3*3];
uvArray = new Vertex[mesh->mNumFaces*3*2];

for (unsigned int i = 0; i < mesh->mNumFaces; i++){
  const aiFace& face = mesh->mFaces[i];
  for(int j=0; j<3; j++){
    aiVector3d uv = mesh->mTextureCoords[0][face.mIndices[j]];
    memcpy(uvArray, &uv, sizeof(Vertex)*2);
    uvArray+=2;

    aiVector3d normal = mesh->mNormals[0][face.mIndices[j]];
    memcpy(normalArray, &normal, sizeof(Vertex)*3);
    normalArray+=3;

    aiVector3d pos = mesh->mVerticess[0][face.mIndices[j]];
    memcpy(vertexArray, &pos, sizeof(Vertex)*3);
    vertexArray+=3;
  }

}

uvArray -=mesh->mNumFaces*3*2;
normalArray-=mesh->mNumFaces*3*3;
vertexArray-=mesh->mNumFaces*3*3;
#endif
