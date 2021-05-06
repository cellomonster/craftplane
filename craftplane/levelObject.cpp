#include "levelObject.h"

#include "mesher.h"
#include "shader.h"

static float maxVertSize = 4096;

LevelObject::LevelObject(Paths shape, glm::vec3 pos, float zThick, 
	unsigned int shadID, unsigned int texID) {

	this->shape = shape;
	this->zThick = zThick;
	this->shadID = shadID;
	this->texID = texID;

	this->transform = glm::mat4(1);
	this->transform = glm::translate(transform, pos);

	this->transLoc = glGetUniformLocation(shadID, "trans");

	pathsToMesh(this->shape, verts, tris, uvs, norms, this->zThick);
	triCount = tris.size();

	glGenVertexArrays(1, &vertAO);
	unsigned int buffers[4];
	glGenBuffers(4, buffers);
	uvBO = buffers[0];
	vertBO = buffers[1];
	triBO = buffers[2];
	normBO = buffers[3];

	glBindVertexArray(vertAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), &verts[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normBO);
	glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(glm::vec3), &norms[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(int), &tris[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// must be AFTER closing vert array object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void LevelObject::updateMesh() {

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(int), &tris[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), &verts[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normBO);
	glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(glm::vec3), &norms[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void LevelObject::draw() {
	glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transform));

	glBindVertexArray(vertAO);
	glDrawElements(GL_TRIANGLES, triCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

unsigned int LevelObject::getShaderId() {
	return this->shadID;
}

unsigned int LevelObject::getTextureId() {
	return this->texID;
}

// todo: needs to be rewritten once rotation supported
void LevelObject::setPos(glm::vec3 pos) {
	this->transform = glm::translate(glm::mat4(1), pos);
}

void LevelObject::translate(glm::vec3 delta) {
	this->transform = glm::translate(transform, delta);
}

void LevelObject::setShape(Paths shape) {
	this->shape = shape;
	pathsToMesh(shape, verts, tris, uvs, norms, zThick);
	triCount = tris.size();
	updateMesh();
}

Paths LevelObject::getShape() {
	return shape;
}

void LevelObject::setZThick(float zThick) {
	this->zThick = zThick;
	pathsToMesh(shape, verts, tris, uvs, norms, zThick);
	updateMesh();
}

glm::vec2 LevelObject::positionAsLocal(glm::vec2 point) {
	glm::vec4 pos(point.xy, 0, 1);
	return (glm::inverse(this->transform) * pos).xy;
}