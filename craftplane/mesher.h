#pragma once

#define GLM_SWIZZLE 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <polyclipping/clipper.hpp>
using namespace ClipperLib;


const float res = 10000;

std::vector<glm::vec3> computeNormals(const std::vector<glm::vec3> &verts,
	const std::vector<unsigned int> &tris);

void pathsToMesh(const Paths &paths,
	std::vector<glm::vec3> &verts,
	std::vector<unsigned int> &tris,
	std::vector<glm::vec2> &uvs,
	std::vector<glm::vec3> &norms,
	const float zThick);