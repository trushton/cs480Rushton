// Parts of this code are taken from http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

#ifndef MESH_H
#define	MESH_H
#include <FreeImagePlus.h>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include "vertex.h"

class Mesh {

    public:
    void Init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices);
    void bindTexture(unsigned char* bytes, long imageSize);

    GLuint VB;
    GLuint IB;
    GLuint TB;
    unsigned int NumIndices;
    unsigned int MaterialIndex;
    unsigned int imageWidth;
    unsigned int imageHeight;
    fipImage image;
};


#endif	/* MESH_H */

