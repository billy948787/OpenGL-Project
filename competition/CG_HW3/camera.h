#ifndef CAMERA_H
#define CAMERA_H

#include "headers.h"

// Camera Declarations.
class Camera {
public:
	// Camera Public Methods.
	Camera(const float aspectRatio);
	Camera(const glm::vec3 pos, const glm::vec3 target, const glm::vec3 up,
		const float fovyInDegree, const float aspectRatio, const float zNear, const float zFar);
	~Camera();

	void setAspectRatio(const float aspectRatio) { this->aspectRatio = aspectRatio; UpdateProjection(fovy, aspectRatio, nearPlane, farPlane); }

	void moveUp(const float delta);
	void moveDown(const float delta);
	void moveRight(const float delta);
	void moveLeft(const float delta);
	void moveBackward(const float delta);
	void moveForward(const float delta);
	void rotate(const float yaw, const float pitch);

	glm::vec3 GetCameraPos() const { return position; }
	glm::mat4x4 GetViewMatrix() const { return viewMatrix; }
	glm::mat4x4 GetProjMatrix() const { return projMatrix; }

	void UpdateView(const glm::vec3 newPos, const glm::vec3 newTarget, const glm::vec3 up);
	void UpdateProjection(const float fovyInDegree, const float aspectRatio, const float zNear, const float zFar);

private:
	// Camera Private Data.
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	glm::vec3 forwardVec;
	
	float fovy;	// in degree.
	float aspectRatio;
	float nearPlane;
	float farPlane;

	glm::mat4x4 viewMatrix;
	glm::mat4x4 projMatrix;
};

#endif
