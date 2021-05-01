#include "camera.h"

Camera::Camera(float fov, int width, int height, float nearClip, float farClip) {
	this->setPerspective(fov, width, height, nearClip, farClip);
}

void Camera::translate(glm::vec3 delta) {
	this->transform = glm::translate(this->transform, delta);
}

void Camera::setPosition(glm::vec3 position) {
	this->transform = glm::translate(glm::mat4(1), position);
}

void Camera::setPerspective(float fov, int width, int height, float nearClip, float farClip) {
	glm::vec3 pos = glm::vec3(0, 1, -8);
	this->transform = glm::lookAt(pos, pos + forward, up);

	float scrRatio = (float)width / (float)height;
	float radFov = glm::radians(fov);
	this->projection = glm::perspective(radFov, scrRatio, nearClip, farClip);
}

glm::mat4 Camera::getProjection() {
	return this->projection;
}

glm::mat4 Camera::getTransform() {
	return this->transform;
}