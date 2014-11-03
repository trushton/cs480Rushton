// Reference: http://www.codecolony.de/opengl.htm#Camera2

#define PI 3.1415926f
#define PIdiv180 (PI/180.0f)
#include <glm/glm.hpp>

class Camera
{
public:
	
	glm::vec3 ViewDir;
	glm::vec3 RightVector;	
	glm::vec3 UpVector;
	glm::vec3 Position;

	GLfloat RotatedX, RotatedY, RotatedZ;	

	GLfloat movementSpeed;
	
	Camera();
	void Move ( glm::vec3 Direction );
	void RotateX ( GLfloat Angle );
	void RotateY ( GLfloat Angle );
	void RotateZ ( GLfloat Angle );

	void MoveForward ( GLfloat Distance );
	void MoveUpward ( GLfloat Distance );
	void StrafeRight ( GLfloat Distance );
};


