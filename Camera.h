// ============================================================================
//
// Camera.h
// -----------------------------------
//
// CAMERA HEADER FILE
//
// The camera Class is used to handle movement around a rendered scene from a 
// camera's point of view.
//
// ============================================================================

#pragma once

// Libraries
#include <vector>
#include "GL\glew.h"

// Include GLM libraries
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

using namespace std;
using namespace glm;

// Camera positioning settings
const GLfloat YAW			= radians(-90.0f);
const GLfloat PITCH			= radians(0.0f);
const GLfloat ZOOM			= radians(45.0f);
const GLfloat SPEED			= 8.0f;
const GLfloat SENSITIVITY	= 0.005f;

// Camera movement directions
enum camera_movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Camera class
class Camera {
private:
	void UpdateCameraVectors();

public:
	// Camera variables
	vec3 position;
	vec3 front, up, right;
	vec3 worldUp;
	GLfloat camYaw;
	GLfloat camPitch;
	GLfloat moveSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	// Constructors
	Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), 
		GLfloat camYaw = YAW, GLfloat camPitch = PITCH);
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, 
		GLfloat inYaw, GLfloat inPitch);

	// Function prototypes
	mat4 GetViewMatrix();
	void ProcessKeyboard(camera_movement direction, GLfloat deltaTime);
	void ProcessMouseMovement(GLfloat offsetX, GLfloat offsetY, GLboolean pitchLimit = true);
	void ProcessMouseScroll(GLfloat offsetY);
};

// Constructor if given a vector
Camera::Camera(vec3 position, vec3 up,GLfloat inYaw, GLfloat inPitch) {
	front = vec3(0.0f, 0.0f, -1.0f);
	moveSpeed = SPEED;
	mouseSensitivity = SENSITIVITY;
	zoom = ZOOM;

	this->position = position;
	this->worldUp = up;
	this->camYaw = inYaw;
	this->camPitch = inPitch;
	this->UpdateCameraVectors();
}

// Constructor if given multiple scalar values
Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, 
			   GLfloat upZ, GLfloat inYaw, GLfloat inPitch) {
	front = vec3(0.0f, 0.0f, -1.0f);
	moveSpeed = SPEED;
	mouseSensitivity = SENSITIVITY;
	zoom = ZOOM;

	this->position = vec3(posX, posY, posZ);
	this->worldUp = vec3(upX, upY, upZ);
	this->camYaw = inYaw;
	this->camPitch = inPitch;
	this->UpdateCameraVectors();
}

// Return view matrix based on camera angle
mat4 Camera::GetViewMatrix() {
	return lookAt(this->position, this->position + this->front, this->up);
}

// Process movement based on keyboard press
void Camera::ProcessKeyboard(camera_movement direction, GLfloat delta_time) {
	GLfloat velocity = this->moveSpeed * delta_time;
	if(direction == FORWARD)
		this->position += this->front * velocity;
	if(direction == BACKWARD)
		this->position -= this->front * velocity;
	if(direction == LEFT)
		this->position -= this->right * velocity;
	if(direction == RIGHT)
		this->position += this->right * velocity;
}

// Process camera angle depending on mouse movement
void Camera::ProcessMouseMovement(GLfloat offsetX, GLfloat offsetY, GLboolean pitchLimit) {
	// Move camera based on sensitivity constant
	offsetX *= this->mouseSensitivity;
	offsetY *= this->mouseSensitivity;
	this->camYaw += offsetX;
	this->camPitch += offsetY;

	// Limit pitch so camera can't go "upside down"
	if(pitchLimit) {
		if(camPitch > radians(89.0f))
			camPitch = radians(89.0f);
		if(camPitch < radians(-89.0f))
			camPitch = radians(-89.0f);
	}

	// Update
	this->UpdateCameraVectors();
}

// Process zoom depending on scroll whell
void Camera::ProcessMouseScroll(GLfloat offsetY) {
	if(zoom >= radians(1.0f) && zoom <= radians(45.0f))
		zoom -= 0.05f* offsetY;
	if(zoom <= radians(1.0f)) 
		zoom = radians(1.0f);
	if(zoom >= radians(45.0f))
		zoom = radians(45.0f);
}

// Recalculates the camera relative vectors whenever it needs updating
void Camera::UpdateCameraVectors() {
	// Calculate the front vector
	vec3 front;
	front.x = (cos(camYaw) * cos(camPitch));
	front.y = sin(camPitch);
	front.z = sin(camYaw) * cos(camPitch);

	// Calculate other vectors based on front vector
	this->front = normalize(front);
	this->right = normalize(cross(this->front, this->worldUp));
	this->up = normalize(cross(this->right, this->front));
}