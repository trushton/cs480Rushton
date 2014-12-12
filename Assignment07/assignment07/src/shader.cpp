#include "shader.h"
#include <fstream>

shader::shader (GLenum shaderType) {
	type = shaderType;
	iShader = glCreateShader(shaderType);
}	

bool shader::initialize (string filename) {
	ifstream fstream;
	string line;
	string shaderStr = "";

	// read in shader data from a file
	fstream.open(filename);
	if (fstream.is_open()) {

		while (getline(fstream, line)) {
			shaderStr += line;
		}
		fstream.close();
		
		const char* shaderData = shaderStr.c_str();

        //compile the shader
        char buffer[512];
		GLint shader_status;
		glShaderSource(iShader, 1, &shaderData, NULL);
		glCompileShader(iShader);
    	glGetShaderInfoLog(iShader, 512, NULL, buffer);

    	//check the compile status
    	glGetShaderiv(iShader, GL_COMPILE_STATUS, &shader_status);
    	if(!shader_status) {

    		if (type == GL_VERTEX_SHADER)
        		cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << endl;
        	else
        		cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << endl;

        	for (int i = 0; i < 512; i++)
        		cout << buffer[i];
        	cout << endl;

        	return false;
    	}
    	return true;
	}
	else {
		cout << "Could not open shader file: " << filename << endl;
		return false;
	}
}

GLuint shader::getShader () {
	return iShader;
}
