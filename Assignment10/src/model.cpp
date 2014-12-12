// Parts of this code are taken from http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

#include "model.h"

Model::Model()
{
}

Model::~Model()
{
    Clear();
}

void Model::Clear()
{
}

bool Model::LoadMesh(const std::string& Filename)
{
    // Release the previously loaded mesh (if it exists)
    Clear();
    
    bool Ret = false;
    Assimp::Importer Importer;

    // load the object file
    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate);
    
    if (pScene) {
        Ret = InitFromScene(pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    return Ret;
}

bool Model::InitFromScene(const aiScene* pScene, const std::string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    aiColor3D color(0.0f, 0.0f, 0.0f);

    // Initialize the meshes in the scene one by one
    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {
        aiMesh* paiMesh = pScene->mMeshes[i];

        // retrieve the material corresponding to this mesh
        aiMaterial* pMaterial = pScene->mMaterials[paiMesh->mMaterialIndex];
        pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);

        InitMesh(pScene, i, color);
    }

    return true;
}


void  Model::InitMesh(const aiScene* pScene, unsigned int Index, aiColor3D color)
{
    m_Entries[Index].MaterialIndex = pScene->mMeshes[Index]->mMaterialIndex;
    
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // initialize all the vertices in the mesh
    for (unsigned int i = 0 ; i < pScene->mMeshes[Index]->mNumVertices ; i++) {
        const aiVector3D* pPos = &(pScene->mMeshes[Index]->mVertices[i]);
        Vertex v;

        v.position[0] = pPos->x;
        v.position[1] = pPos->y;
        v.position[2] = pPos->z;

        // check if there is a texture for this mesh
        if (pScene->mMeshes[Index]->HasTextureCoords(0)) {
            aiVector3D textureCoord = pScene->mMeshes[Index]->mTextureCoords[0][i];
            v.uv[0] = textureCoord.x;
            v.uv[1] = textureCoord.y;  
        }

        if (pScene->mMeshes[Index]->HasNormals()) {
            const aiVector3D* normal = &(pScene->mMeshes[Index]->mNormals[i]);
            v.normal[0] = normal->x;
            v.normal[1] = normal->y;
            v.normal[2] = normal->z; 
        }

        Vertices.push_back(v);
    }

    // store the indices of the faces in the mesh
    for (unsigned int i = 0 ; i < pScene->mMeshes[Index]->mNumFaces; i++) {
        const aiFace& Face = pScene->mMeshes[Index]->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }

    // get texture
    InitMaterials(pScene, Index);

    // bind buffers
    m_Entries[Index].Init(Vertices, Indices);
}


void Model::InitMaterials(const aiScene* pScene, unsigned meshIndex)
{
    aiString Path;
    const aiMaterial* pMaterial = pScene->mMaterials[m_Entries[meshIndex].MaterialIndex];

    // check if there are textures
    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {

            // load the texture image
            m_Entries[meshIndex].image.load(Path.C_Str());
            m_Entries[meshIndex].image.convertTo32Bits();

            // bind the texture data buffer
            glGenTextures(1, &m_Entries[meshIndex].TB);
            glBindTexture(GL_TEXTURE_2D, m_Entries[meshIndex].TB);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Entries[meshIndex].image.getWidth(), m_Entries[meshIndex].image.getHeight(), 
                0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)m_Entries[meshIndex].image.accessPixels());

            // specify sampling method
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
    }
}

void Model::Render(glm::mat4 mvp, glm::mat4 model, LightingEngine lightingEngine)
{
    // set light values on or off
    lightingEngine.toggleAmbientLight();
    lightingEngine.toggleDiffuseLight();

    lightingEngine.toggleSpecularLight();
    lightingEngine.togglePointLight();
    lightingEngine.toggleSpotLight();

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(lightingEngine.loc_vertexPosition);
    glEnableVertexAttribArray(lightingEngine.loc_vertexTexture);
    glEnableVertexAttribArray(lightingEngine.loc_vertexNormal);

    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {

        lightingEngine.setMVP(mvp);
        lightingEngine.setModelMatrix(model);
        lightingEngine.setCameraPos(glm::vec3(0.0f, 8.0f, -16.0f));
        lightingEngine.setMatSpecularIntensity();
        lightingEngine.setMatSpecularPower(32.0f);

        // directional light
        lightingEngine.setDirectionalLight(lightingEngine.directLight);

        // point lights
        lightingEngine.setPointLights(1, lightingEngine.pointLights);

        // spot lights
        lightingEngine.setSpotLights(1, lightingEngine.spotLights);

        //set pointers into the vbo for each of the attributes(position and color)
        glVertexAttribPointer(lightingEngine.loc_vertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer(lightingEngine.loc_vertexTexture, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,uv));
        glVertexAttribPointer(lightingEngine.loc_vertexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        // bind vertex and index buffers for this mesh
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].VB);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IB);

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Entries[i].TB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Entries[i].image.getWidth(), m_Entries[i].image.getHeight(), 
            0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)m_Entries[i].image.accessPixels());

        // draw elements by using indexing
        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

    // clean up
    glDisableVertexAttribArray(lightingEngine.loc_vertexPosition);
    glDisableVertexAttribArray(lightingEngine.loc_vertexTexture);
    glDisableVertexAttribArray(lightingEngine.loc_vertexNormal);
}
