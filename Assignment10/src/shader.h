#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

using namespace std;

class shader {

	public:
	shader(GLenum);
	bool initialize(string);
	
	GLuint getShader ();

	private:
	GLenum type;
	GLuint iShader;
};

#endif // SHADER_H