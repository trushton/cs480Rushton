// Parts of this code are taken from http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

#include <assert.h>
#include "mesh.h"

void Mesh::Init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices)
{
    NumIndices = Indices.size();

    // initialize vertex buffer
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    // initialize index buffer
    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &Indices[0], GL_STATIC_DRAW);
}

void Mesh::bindTexture(unsigned char* bytes, long imageSize) {

    glGenTextures(1, &TB);
    glBindTexture(GL_TEXTURE_2D, TB);
}
