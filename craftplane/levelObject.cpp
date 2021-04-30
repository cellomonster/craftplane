#include "levelObject.h"

#include "shader.h"

#include <polyclipping/clipper.hpp>
#include <mapbox/earcut.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>

namespace mapbox {
	namespace util {
		template <>
		struct nth<0, IntPoint> {
			inline static auto get(const IntPoint& t) {
				return t.X / res;
			};
		};
		template <>
		struct nth<1, IntPoint> {
			inline static auto get(const IntPoint& t) {
				return t.Y / res;
			};
		};

		template <>
		struct nth<0, glm::vec2> {
			inline static auto get(const glm::vec2& t) {
				return t.x;
			};
		};
		template <>
		struct nth<1, glm::vec2> {
			inline static auto get(const glm::vec2& t) {
				return t.y;
			};
		};
	}
}

LevelObject::LevelObject(Paths shape, glm::vec3 pos, float zThick) {

	this->shape = shape;
	this->zThick = zThick;

	transform = glm::mat4(1);
	transform = glm::translate(transform, pos);

	shader = Shader("stdvert.vert", "stdfrag.frag");

	viewLoc = glGetUniformLocation(shader.getID(), "view");
	transLoc = glGetUniformLocation(shader.getID(), "trans");
	projLoc = glGetUniformLocation(shader.getID(), "proj");

	pathsToMesh(shape, verts, tris, uvs, norms, zThick);

	glGenVertexArrays(1, &vertAO);
	glGenBuffers(1, &uvBO);
	glGenBuffers(1, &vertBO);
	glGenBuffers(1, &triBO);
	glGenBuffers(1, &normBO);

	glBindVertexArray(vertAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normBO);
	glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(glm::vec3), &norms[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(int), &tris[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// must be AFTER closing vert array object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void LevelObject::draw(glm::mat4 camTransform, glm::mat4 projection) {
	shader.use();

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camTransform));
	glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vertAO);
	glDrawElements(GL_TRIANGLES, tris.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

IntPoint vec2ToIntpoint(glm::vec2 vec) {
	return IntPoint(vec.x * res, vec.y * res);
}

glm::vec2 intpointToVec2(IntPoint point) {
	return glm::vec2(point.X / res, point.Y / res);
}

glm::vec3 vec2toVec3(glm::vec2 vec, float z) {
	return glm::vec3(vec.x, vec.y, z);
}

static void pathsToMesh(const Paths paths,
	std::vector<glm::vec3> &verts,
	std::vector<unsigned int> &tris,
	std::vector<glm::vec2> &uvs,
	std::vector<glm::vec3> &norms,
	float zThick) {

	// placeholder bevel 
	glm::vec2 bp[] = {
		glm::vec2(0.1f, 0),
		glm::vec2(0, 0.1f),
		glm::vec2(0, 1),
	};
	int bpSize = 3;

	bool s[] = {
		false,
		true,
		false,
	};

	std::vector<std::vector<glm::vec2>> offsetPaths(paths.size(), std::vector<glm::vec2>());

	int k = 0;

	//bevels
	for (int l = 0; l < paths.size(); l++)
	{
		Path path = paths[l];
		std::vector<glm::vec2> offsetPath(path.size(), glm::vec2());

		int c = bpSize;

		for (int i = 0; i < bpSize; i++) {
			if (s[i])
				c++;
		}

		//bevels

		//front face offset inward
		float o = bp[0].x;

		for (int i = 0; i < path.size(); i++)
		{
			//front face vertex position
			glm::vec3 v = vec2toVec3(intpointToVec2(path[i]), 0);

			//next index
			int j = (i + 1) % path.size();

			//normal A
			glm::vec2 na = intpointToVec2(IntPoint(path[i].Y - path[j].Y, path[j].X - path[i].X));
			na = glm::normalize(na);

			//previous index
			j = (i - 1);
			if (j < 0)
				j += path.size();

			//normal B
			glm::vec2 nb = intpointToVec2(IntPoint(path[j].Y - path[i].Y, path[i].X - path[j].X));
			nb = glm::normalize(nb);

			//bevel direction and magnitude
			glm::vec3 b = vec2toVec3(glm::normalize(na + nb), 0);
			b *= 1.41421f / sqrt(1 + glm::dot(na, nb));

			v += b * bp[0].x;

			offsetPath[i] = v;

			float angle = glm::degrees(acos(dot(na, nb)));
			bool sh = angle > 30;

			if (sh)

				for (int n = 0; n < bpSize - 1; n++)
				{
					//z offset
					glm::vec3 z(0, 0, bp[n].y);
					glm::vec3 z2;
					if (n == bpSize - 2)
						z2 = glm::vec3(0, 0, zThick);
					else
						z2 = glm::vec3(0, 0, bp[n + 1].y);

					glm::vec3 nbv3 = vec2toVec3(nbv3, 0);
					glm::vec3 nn = v + nbv3 * (z.z - z2.z);
					//
					// int a = 1;
					//
					// if (s[n + 1])
					verts.push_back(v + b * (bp[n].x - o) + z);

					uvs.push_back(verts[verts.size() - 1]);

					if (n == bpSize - 2)
					{
						verts.push_back(v + b * (bp[n + 1].x - o) + z2);
						uvs.push_back(nn);
					}
					else if (s[n + 1])
					{
						verts.push_back(v + b * (bp[n + 1].x - o) + z2);
						uvs.push_back(verts[verts.size() - 1]);
					}
				}

			int a = 0;

			for (int n = 0; n < bpSize - 1; n++)
			{
				//z offset
				glm::vec3 z(0, 0, bp[n].y);
				glm::vec3 z2;
				if (n == bpSize - 2)
					z2 = glm::vec3(0, 0, zThick);
				else
					z2 = glm::vec3(0, 0, bp[n + 1].y);

				glm::vec3 nn;
				if (sh)
					nn = v + vec2toVec3(na, 0) * (z.z - z2.z);
				else
				{
					nn = v + glm::normalize(b) * (z.z - z2.z);
				}

				if (s[n])
					a++;

				if (i == path.size() - 1)
				{
					tris.push_back(k + n + a);
					tris.push_back(verts.size() + 1);
					tris.push_back(verts.size());

					tris.push_back(verts.size() + 1);
					tris.push_back(k + n + a);
					tris.push_back(k + n + a + 1);
				}
				else
				{
					tris.push_back(verts.size() + c);
					tris.push_back(verts.size() + 1);
					tris.push_back(verts.size());

					tris.push_back(verts.size() + 1);
					tris.push_back(verts.size() + c);
					tris.push_back(verts.size() + c + 1);
				}

				//add vertices
				verts.push_back(v + b * (bp[n].x - o) + z);
				uvs.push_back(verts[verts.size()- 1]);

				if (n == bpSize - 2)
				{
					verts.push_back(v + b * (bp[n + 1].x - o) + z2);
					uvs.push_back(nn);
				}
				else if (s[n + 1])
				{
					verts.push_back(v + b * (bp[n + 1].x - o) + z2);
					uvs.push_back(verts[verts.size() - 1]);
				}
			}
		}

		offsetPaths[l] = offsetPath;

		k = verts.size();
	}

	computeNormals(verts, tris, norms);

	// front face

	for (unsigned int i = 0; i < offsetPaths[0].size(); i++) {
		verts.push_back(vec2toVec3(offsetPaths[0][i], 0));
		uvs.push_back(offsetPaths[0][i]);
		norms.push_back(glm::vec3(0, 0, -1));
	}

	std::vector<unsigned int> frontTris = mapbox::earcut<unsigned int>(paths);
	for (unsigned int i = 0; i < frontTris.size(); i++) {
		tris.push_back(frontTris[i] + k);
	}
}

static void computeNormals(const std::vector<glm::vec3> verts,
	const std::vector<unsigned int> tris,
	std::vector<glm::vec3> &norms) {

	norms = std::vector<glm::vec3>(verts.size(), glm::vec3());

	// cross products
	for (unsigned int i = 0; i < tris.size(); i+= 3) {	
		glm::vec3 v1 = verts[tris[i]] - verts[tris[i + 1]];
		glm::vec3 v2 = verts[tris[i + 1]] - verts[tris[i + 2]];
		norms[tris[i + 1]] += glm::cross(v1, v2);

		v1 = verts[tris[i + 1]] - verts[tris[i + 2]];
		v2 = verts[tris[i + 2]] - verts[tris[i]];
		norms[tris[i + 2]] += glm::cross(v1, v2);

		v1 = verts[tris[i + 2]] - verts[tris[i]];
		v2 = verts[tris[i]] - verts[tris[i + 1]];
		norms[tris[i]] += glm::cross(v1, v2);
	}


	// normalize all
	for (int i = 0; i < norms.size(); i++) {
		norms[i] = glm::normalize(norms[i]);
	}
}