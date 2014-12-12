#include <iostream>
#include <string>

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
