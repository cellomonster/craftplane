#include <iostream>

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

float screenRatio = (float)windowWidth / (float)windowHeight;
float deltaTime = 0;
float lastFrame = 0;

unsigned int stdShaderId;

Camera cam(fov, windowHeight, windowHeight, nearClip, farClip);
std::vector<LevelObject> levelObjects;

LevelObject* buildingObject;
Paths* buildingPaths;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	cam.setPerspective(fov, width, height, nearClip, farClip);
}

/*void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	float xoffset = xpos - mouseLastX;
	float yoffset = mouseLastY - ypos;
	mouseLastX = xpos;
	mouseLastY = ypos;
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	cursorPos += glm::vec2(xoffset, yoffset);
	std::cout << cursorPos.x << std::endl;
	std::cout << cursorPos.y << std::endl;
	cursor->translate(glm::vec3(cursorPos.x, cursorPos.y, 0));

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	float radYaw = glm::radians(yaw);
	float radPitch = glm::radians(pitch);
	direction.x = cos(radYaw) * cos(radPitch);
	direction.y = sin(radPitch);
	direction.z = sin(radYaw) * cos(radPitch);
	camFront = glm::normalize(direction);
}*/

void calc_delta_time() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void processKeys(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//build
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {

		glm::mat4 camTransform = cam.getTransform();
		glm::vec3 camPos(camTransform[3].x, -camTransform[3].y, 0);

		buildingPaths = new Paths{
			Path{
			IntPoint(0, 0),
			IntPoint(20000, 0),
			IntPoint(20000, 20000),
			IntPoint(0, 20000),}
		};
		buildingObject = new LevelObject(*buildingPaths,
			camPos, 1.5f, stdShaderId, 0);

		levelObjects.push_back(*buildingObject);
	}
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
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Couldn't init GLAD!" << std::endl;
		return -1;
	}
	framebuffer_size_callback(window, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, processKeys);
	//glfwSetCursorPosCallback(window, mouse_callback);

	// get texture
	int w, h, nrChannels;
	unsigned char* data = stbi_load("wood.jpg", &w, &h, &nrChannels, 0);
	if (!data) {
		std::cout << "Couldn't load texture!" << std::endl;
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
	levelObjects.push_back(LevelObject(p1, glm::vec3(0, 0, 0), 1, stdShaderId, tex));

	Paths p2{
		Path{
			IntPoint(100000, 0),
			IntPoint(-100000, 0),
			IntPoint(-100000, -10000),
			IntPoint(100000, -10000),
		}
	};
	levelObjects.push_back(LevelObject(p2, glm::vec3(0, 0, -2.5F), 5, stdShaderId, tex));

	Paths p3{
		Path{
			IntPoint(0, 20000),
			IntPoint(20000, 20000),
			IntPoint(10000, 30000),
		}
	};
	levelObjects.push_back(LevelObject(p3, glm::vec3(0, 0, -0.1f), 1.2f, stdShaderId, tex));

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
	levelObjects.push_back(LevelObject(p4, glm::vec3(3, 0, -1.5F), 0.5f, stdShaderId, tex));

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
	levelObjects.push_back(LevelObject(p5, glm::vec3(3, 0, -1.7f), 1.1f, stdShaderId, tex));

	//draw settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
		glClearColor(0.2f, 0.3f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getTransform()));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProjection()));

		for (int i = 0; i < levelObjects.size(); i++) {
			levelObjects[i].draw();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// termination
	glfwTerminate();
	return 0;
}