#include "camera.h"

Camera::Camera(const float aspectRatio)
{
	// Default camera pose and parameters.
	position = glm::vec3(0.0f, 0.0f, 3.0f);
	target = glm::vec3(0.0f, 0.0f, 0.0f);
	fovy = 45.0f;
	nearPlane = 0.1f;
	farPlane = 1000.0f;
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	UpdateView(position, target, up);
	UpdateProjection(fovy, aspectRatio, nearPlane, farPlane);
}

Camera::Camera(const glm::vec3 pos, const glm::vec3 target, const glm::vec3 up,
	const float fovyInDegree, const float aspectRatio, const float zNear, const float zFar)
{
	position = pos;
	this->target = target;
	fovy = fovyInDegree;
	nearPlane = zNear;
	farPlane = zFar;
	this->up = up;
	this->aspectRatio = aspectRatio;

	forwardVec = glm::normalize(target - position);

	UpdateView(position, target, up);
	UpdateProjection(fovy, aspectRatio, nearPlane, farPlane);
}

Camera::~Camera()
{}

void Camera::UpdateView(const glm::vec3 newPos, const glm::vec3 newTarget, const glm::vec3 up)
{
	position = newPos;
	target = newTarget;
	viewMatrix = glm::lookAt(position, target, up);
}

void Camera::UpdateProjection(const float fovyInDegree, const float aspectRatio, const float zNear, const float zFar)
{
	fovy = fovyInDegree;
	nearPlane = zNear;
	farPlane = zFar;
	projMatrix = glm::perspective(glm::radians(fovyInDegree), aspectRatio, nearPlane, farPlane);
}

void Camera::moveUp(const float delta)
{
	position += delta * glm::normalize(glm::cross(glm::normalize(target - position), up));
	UpdateView(position, target, up);
}

void Camera::moveDown(const float delta)
{
	position -= delta * glm::normalize(glm::cross(glm::normalize(target - position), up));
	UpdateView(position, target, up);
}

void Camera::moveRight(const float delta)
{
	position += delta * glm::normalize(glm::cross(glm::normalize(target - position), up));
	UpdateView(position, target, up);
}

void Camera::moveLeft(const float delta)
{
	position -= delta * glm::normalize(glm::cross(glm::normalize(target - position), up));
	UpdateView(position, target, up);
}

void Camera::moveBackward(const float delta)
{
	position -= delta * forwardVec;
	forwardVec = glm::normalize(target - position);
	UpdateView(position, target, up);
}

void Camera::moveForward(const float delta)
{
	position += delta * forwardVec;
	UpdateView(position, target, up);
}

void Camera::rotate(const float yaw, const float pitch)
{
	glm::vec3 front = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(front, up));
	glm::vec3 newFront = glm::normalize(glm::rotate(glm::mat4(1.0f), glm::radians(yaw), up) * glm::vec4(front, 1.0f));
	newFront = glm::normalize(glm::rotate(glm::mat4(1.0f), glm::radians(pitch), right) * glm::vec4(newFront, 1.0f));
	target = position + newFront;
	UpdateView(position, target, up);
}