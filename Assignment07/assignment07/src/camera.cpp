// Reference: http://www.codecolony.de/opengl.htm#Camera2

#include "camera.h"
#include "math.h"
#include <iostream>

Camera::Camera()
{
	// initialize view
	Position.x = 0.0f;
	Position.y = 182.133f;
	Position.z = 370.293f;
	ViewDir.x = 0.0f;
	ViewDir.y = -0.48f;
	ViewDir.z = -0.87f;
	RightVector.x = 1.0f;
	RightVector.y = 0.0f;
	RightVector.z = 0.0f;
	UpVector.x = 0.0f;
	UpVector.y = 0.819152f;
	UpVector.z = 0.573576f;
	RotatedX = RotatedY = RotatedZ = 0.0f;
	movementSpeed = 1.0f;
}

void Camera::Move (glm::vec3 Direction)
{
	cout << movementSpeed << endl;
	Position = (Position + Direction) * movementSpeed;
}

void Camera::RotateX (GLfloat Angle)
{
	RotatedX += Angle;

	// Rotate viewdir around the right vector:
	ViewDir = glm::normalize(ViewDir*cos(Angle*PIdiv180) + UpVector*sin(Angle*PIdiv180));

	// now compute the new UpVector (by cross product)
	UpVector = glm::cross(ViewDir, RightVector)*-1.0f;
}

void Camera::RotateY (GLfloat Angle)
{
	RotatedY += Angle;

	ViewDir = glm::normalize(ViewDir*cos(Angle*PIdiv180)- RightVector*sin(Angle*PIdiv180));

	RightVector = glm::cross(ViewDir, UpVector);
}

void Camera::RotateZ (GLfloat Angle)
{
	RotatedZ += Angle;

	RightVector = glm::normalize(RightVector*cos(Angle*PIdiv180) + UpVector*sin(Angle*PIdiv180));

	//now compute the new UpVector (by cross product)
	UpVector = glm::cross(ViewDir, RightVector)*-1.0f;
}

void Camera::MoveForward( GLfloat Distance )
{
	cout << movementSpeed << endl;
	Position = Position + (ViewDir*-(Distance * movementSpeed));
}

void Camera::StrafeRight ( GLfloat Distance )
{
	cout << movementSpeed << endl;
	Position = Position + (RightVector*(Distance* movementSpeed));
}

void Camera::MoveUpward( GLfloat Distance )
{
	cout << movementSpeed << endl;
	Position = Position + (UpVector*(Distance * movementSpeed));
}
