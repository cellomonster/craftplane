#pragma once

#include <polyclipping/clipper.hpp>
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>

using namespace ClipperLib;

const float res = 10000;

class LevelObject
{
private:
	glm::mat4 transform;

	Paths shape;
	Shader shader;

	std::vector<glm::vec3> verts;
	std::vector<unsigned int> tris;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> norms;

	unsigned int viewLoc;
	unsigned int transLoc;
	unsigned int projLoc;

	float zPos;
	float zThick;

	unsigned int uvBO, vertBO, vertAO, triBO, normBO;

public:
	LevelObject(Paths shape, glm::vec3 pos, float zThick);
	void draw(glm::mat4 camView, glm::mat4 projection);
};

static void pathsToMesh(const Paths paths,
	std::vector<glm::vec3>& verts,
	std::vector<unsigned int>& tris,
	std::vector<glm::vec2>& uvs,
	std::vector<glm::vec3>& norms,
	float zThick);

static void computeNormals(const std::vector<glm::vec3> verts,
	const std::vector<unsigned int> tris,
	std::vector<glm::vec3>& norms);
