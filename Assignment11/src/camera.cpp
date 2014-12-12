// Reference: http://www.codecolony.de/opengl.htm#Camera2

#include "camera.h"
#include <cmath>
#include <iostream>

using std::cout;
using std::endl;

Camera::Camera()
{
	// initialize view
	Position.x = -40.0f;
	Position.y = 45.0f;
	Position.z = 0.0f;
	ViewDir.x = 0.7f;
	ViewDir.y = -0.7f;
	ViewDir.z = -0.00f;
	RightVector.x = 1.0f;
	RightVector.y = 0.0f;
	RightVector.z = 0.0f;
	UpVector.x = 0.70f;
	UpVector.y = 0.70f;
	UpVector.z = 0.0f;
	RotatedX = RotatedY = RotatedZ = 0.0f;
	movementSpeed = 1.0f;
}

void Camera::Move (glm::vec3 Direction)
{
	Position = (Position + Direction) * movementSpeed;

}

void Camera::RotateX (GLfloat Angle)
{
	RotatedX += Angle;

	// Rotate viewdir around the right vector:
	ViewDir = glm::normalize(ViewDir*(float)cos(Angle*PIdiv180) + UpVector*(float)sin(Angle*PIdiv180));

	// now compute the new UpVector (by cross product)
	UpVector = glm::cross(ViewDir, RightVector)*-1.0f;
}

void Camera::RotateY (GLfloat Angle)
{
	RotatedY += Angle;

	ViewDir = glm::normalize(ViewDir*(float)cos(Angle*PIdiv180)- RightVector*(float)sin(Angle*PIdiv180));

	RightVector = glm::cross(ViewDir, UpVector);
}

void Camera::RotateZ (GLfloat Angle)
{
	RotatedZ += Angle;

	RightVector = glm::normalize((RightVector*(float)cos(Angle*PIdiv180)) + (UpVector*(float)sin(Angle*PIdiv180)));

	//now compute the new UpVector (by cross product)
	UpVector = glm::cross(ViewDir, RightVector)*-1.0f;
}

void Camera::MoveForward( GLfloat Distance )
{

	Position = Position + (ViewDir*-(Distance * movementSpeed));
		if(Position.x >= 35)
			Position.x = 35;
		if(Position.x <= -35)
			Position.x = -35;
	//	if(Position.y >= 35)
			//Position.y = 35;
		if(Position.y <= 5)
			Position.y = 5;
}

void Camera::StrafeRight ( GLfloat Distance )
{

	Position = Position + (RightVector*(Distance* movementSpeed));
		if(Position.x >= 35)
			Position.x = 35;
		if(Position.x <= -35)
			Position.x = -35;
		if(Position.y >= 35)
			Position.y = 35;
		if(Position.y <= 5)
			Position.y = 5;
}

void Camera::MoveUpward( GLfloat Distance )
{
	Position = Position + (UpVector*(Distance * movementSpeed));
}
