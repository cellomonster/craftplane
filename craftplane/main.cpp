#include <iostream>

#define GLM_SWIZZLE 
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.h"
#include "levelObject.h"
#include "camera.h"

int windowWidth = 800;
int windowHeight = 800;

const float fov = 50;
const float nearClip = 0.1f;
const float farClip = 10000;

float deltaTime = 0;
float lastFrame = 0;

glm::vec3 mousePosition;

unsigned int stdShaderId;

Camera cam(fov, windowHeight, windowHeight, nearClip, farClip);
std::vector<LevelObject*> levelObjects;

LevelObject* buildingObject;
Paths buildingPaths;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
	cam.setPerspective(fov, width, height, nearClip, farClip);
}

void calc_delta_time() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

bool ctrlPressed;
bool wireframe;
void processKeys(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//build
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
		if (buildingObject == NULL) {

			buildingPaths = Paths{
				Path{
					IntPoint(0, 0),
				}
			};
			buildingObject = new LevelObject(buildingPaths,
				glm::vec3(mousePosition.x, mousePosition.y, 0), 0.5f, stdShaderId, 0);

			levelObjects.push_back(buildingObject);
		}
		else {
			glm::vec2 newPoint = buildingObject->positionAsLocal(mousePosition);
			IntPoint newIPoint(newPoint.x * res, newPoint.y * res);
			(buildingPaths)[0].push_back(newIPoint);
			buildingObject->setShape(buildingPaths);
		}
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		buildingObject = NULL;
	}


	ctrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

	if (ctrlPressed) {
		// wireframe
		if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			wireframe = !wireframe;

			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		// renderdoc capture
		if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
			
		}
	}
}

void mouse_callback(GLFWwindow* window, double x, double y) {
	x = (2.0f * x) / windowWidth - 1.0f;
	y = 1.0f - (2.0f * y) / windowHeight;

	glm::vec2 mousePosScreen(x, -y);
	mousePosition = cam.getCursorPosition(mousePosScreen, 0);
}

void process_inputs(GLFWwindow* window) {

	//move camera
	float camSpeed = 5 * deltaTime;

	glm::vec3 dir;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		dir += up;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		dir -= up;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		dir += right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		dir -= right;

	glm::normalize(dir);
	dir *= -camSpeed;
	cam.translate(dir);
}

int main() {
	// build window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Craftplane", NULL, NULL);
	if (window == NULL) {
		std::cout << "Couldn't create GLFW window!" << std::endl;
		glfwTerminate();
		std::cin.ignore();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Couldn't init GLAD!" << std::endl;
		std::cin.ignore();
		return -1;
	}
	framebuffer_size_callback(window, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, processKeys);
	glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);

	// get texture
	int w, h, nrChannels;
	unsigned char* data = stbi_load("wood.jpg", &w, &h, &nrChannels, 0);
	if (!data) {
		std::cout << "Couldn't load texture!" << std::endl;
		std::cin.ignore();
		return -1;
	}
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	// shader
	Shader stdShader("stdvert.vert", "stdfrag.frag");
	stdShaderId = stdShader.getID();
	glUseProgram(stdShaderId);
	unsigned int viewLoc = glGetUniformLocation(stdShaderId, "view");
	unsigned int projLoc = glGetUniformLocation(stdShaderId, "proj");;

	// create test level object
	Paths p1 {
		Path{
			IntPoint(0, 0),
			IntPoint(20000, 0),
			IntPoint(20000, 20000),
			IntPoint(0, 20000),
		}
	};
	levelObjects.push_back(new LevelObject(p1, glm::vec3(0, 0, 0), 1, stdShaderId, tex));

	Paths p2{
		Path{
			IntPoint(100000, 0),
			IntPoint(-100000, 0),
			IntPoint(-100000, -10000),
			IntPoint(100000, -10000),
		}
	};
	levelObjects.push_back(new LevelObject(p2, glm::vec3(0, 0, -2.5F), 5, stdShaderId, tex));

	Paths p3{
		Path{
			IntPoint(-1000, 20000),
			IntPoint(21000, 20000),
			IntPoint(10000, 30000),
		}
	};
	levelObjects.push_back(new LevelObject(p3, glm::vec3(0, 0, -0.4f), 1.8f, stdShaderId, tex));

	Paths p4{
		Path{
			IntPoint(4800, 0),
			IntPoint(4000, 16600),
			IntPoint(2400, 21000),
			IntPoint(7200, 34000),
			IntPoint(3300, 34000),
			IntPoint(100, 25400),
			IntPoint(-4600, 34100),
			IntPoint(-8100, 34100),
			IntPoint(-1200, 17300),
			IntPoint(-5200, 0),
		},
	};
	levelObjects.push_back(new LevelObject(p4, glm::vec3(3, 0, -1.5F), 0.5f, stdShaderId, tex));

	Paths p5{
		Path{
			IntPoint(14200, 29700),
			IntPoint(22000, 38500),
			IntPoint(6000, 54700),
			IntPoint(-13800, 53200),
			IntPoint(-24300, 39000),
			IntPoint(-18500, 28900),
			IntPoint(-2500, 34200),
		},
	};
	levelObjects.push_back(new LevelObject(p5, glm::vec3(3, 0, -1.7f), 1.1f, stdShaderId, tex));

	//draw settings
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	// intro 
	std::cout << "Welcome to craftplane!" << std::endl;
	//std::cout << "Use the arrow keys to move the cursor" << std::endl;
	//std::cout << "Use the spacebar to start creating a new object or add a new point to an existing object" << std::endl;
	//std::cout << "use enter to finish a new object" << std::endl;

	// game loop
	while (!glfwWindowShouldClose(window)) {

		calc_delta_time();
		process_inputs(window);

		// render
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getTransform()));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProjection()));

		for (int i = 0; i < levelObjects.size(); i++) {
			levelObjects[i]->draw();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// termination
	glfwTerminate();
	return 0;
}