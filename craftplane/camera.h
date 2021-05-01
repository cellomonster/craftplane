#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const glm::vec3 up = glm::vec3(0, 1, 0);
const glm::vec3 forward = glm::vec3(0, 0, 1);
const glm::vec3 right = glm::vec3(1, 0, 0);

class Camera
{
private:
	glm::mat4 transform;
	glm::mat4 projection;
	float fov;
public:
	Camera(float fov, int width, int height, float nearClip, float farClip);

	void translate(glm::vec3 pos);

	void setPosition(glm::vec3 pos);
	void setPerspective(float fov, int width, int height, float nearClip, float farClip);
	
	glm::mat4 getProjection();
	glm::mat4 getTransform();
};

