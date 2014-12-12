// This code is based on the example code provided by this link: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <GL/glut.h> 
#include <vector>
#include "vertex.h"
#include <FreeImagePlus.h>

class Mesh {

    public:

        void setData();
        
        std::vector<textureVertex> vertices;
        std::vector<unsigned int> indices;

        GLuint VB;
        GLuint IB;
        GLuint TB;
        unsigned int numIndices;
        unsigned int materialIndex;

        fipImage image;
};
#endif