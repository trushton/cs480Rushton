#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

struct Vertex{
	GLfloat position[3];
	GLfloat color[3];
	};

int loadOBJ(
	const char * path, 
	std::vector<Vertex> & out_vertices, 
	std::vector<Vertex> & out_uvs, 
	std::vector<Vertex> & out_normals
);

#endif
