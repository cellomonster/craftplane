#pragma once

#include "mesher.h"

#define GLM_SWIZZLE 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <polyclipping/clipper.hpp>

using namespace ClipperLib;

class LevelObject
{
private:
	glm::mat4 transform;

	Paths shape;
	unsigned int shadID;
	unsigned int texID;

	std::vector<glm::vec3> verts;
	std::vector<unsigned int> tris;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> norms;

	unsigned int triCount;

	unsigned int transLoc;

	float zPos;
	float zThick;

	unsigned int vertAO, uvBO, vertBO, triBO, normBO;

	void updateMesh();

public:
	LevelObject(Paths shape, glm::vec3 pos, float zThick,
		unsigned int shadID, unsigned int texID);
	void draw();

	unsigned int getShaderId();
	unsigned int getTextureId();

	void setPos(glm::vec3 pos);
	void translate(glm::vec3 delta);

	void setShape(Paths shape);
	void setZThick(float zThick);
	void addPoint(IntPoint point);
	void addPointWorldSpace(glm::vec2 point);

	glm::vec2 positionAsLocal(glm::vec2 point);
	Paths getShape();
};