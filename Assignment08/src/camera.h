// Reference: http://www.codecolony.de/opengl.htm#Camera2
#ifndef CAMERA_H
#define CAMERA_H

#define PI 3.1415926f
#define PIdiv180 (PI/180.0f)

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/gl.h>

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


#endif // CAMERA_H