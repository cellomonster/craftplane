#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.h"
#include "levelObject.h"

int winW = 800;
int winH = 800;
float scrRatio = winW / winH;

const float mouseSensitivity = 0.1f;
float mouseLastX = winW / 2.0f, mouseLastY = winH / 2.0f;

float deltaTime = 0;
float lastFrame = 0;

const float fov = 50;
const float nearClip = 0.1f;
const float farClip = 10000;

float pitch, yaw = -90;

const glm::vec3 up = glm::vec3(0, 1, 0);
glm::vec3 camPos = glm::vec3(0, 1, -5);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 camRight = glm::normalize(glm::cross(up, camFront));
glm::vec3 camUp = glm::cross(camFront, camRight);
glm::mat4 camTrans = glm::lookAt(camPos, camPos + camFront, up);
glm::mat4 projection;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	winW = width;
	winH = height;
	scrRatio = winW / winH;
	glViewport(0, 0, width, height);
	float radFov = glm::radians(fov);
	projection = glm::perspective(radFov, scrRatio, nearClip, farClip);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	float xoffset = xpos - mouseLastX;
	float yoffset = mouseLastY - ypos;
	mouseLastX = xpos;
	mouseLastY = ypos;
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

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
}

void calc_delta_time() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void process_inputs(GLFWwindow* window) {
	const float camSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camPos += camSpeed * camFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camPos -= camSpeed * camFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;

	camTrans = glm::lookAt(camPos, camPos + camFront, up);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// build window
	GLFWwindow* window = glfwCreateWindow(winW, winH, "Craftplane", NULL, NULL);
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

	framebuffer_size_callback(window, winW, winH);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// lock cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

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
	unsigned int stdShaderId = stdShader.getID();
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
	LevelObject levelObject1 = LevelObject(p1, glm::vec3(0, 0, 0), 1, stdShaderId, tex);

	Paths p2{
		Path{
			IntPoint(100000, 0),
			IntPoint(-100000, 0),
			IntPoint(-100000, -10000),
			IntPoint(100000, -10000),
		}
	};
	LevelObject levelObject2 = LevelObject(p2, glm::vec3(0, 0, -2.5F), 5, stdShaderId, tex);

	Paths p3{
		Path{
			IntPoint(0, 20000),
			IntPoint(20000, 20000),
			IntPoint(10000, 30000),
		}
	};
	LevelObject levelObject3 = LevelObject(p3, glm::vec3(0, 0, -0.1f), 1.2f, stdShaderId, tex);

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
	LevelObject levelObject4 = LevelObject(p4, glm::vec3(3, 0, -1.5F), 0.5f, stdShaderId, tex);

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
	LevelObject levelObject5 = LevelObject(p5, glm::vec3(3, 0, -1.7f), 1.1f, stdShaderId, tex);

	// buffers
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// game loop
	while (!glfwWindowShouldClose(window)) {

		calc_delta_time();

		process_inputs(window);

		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camTrans));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		levelObject1.draw();
		levelObject2.draw();
		levelObject3.draw();
		levelObject4.draw();
		levelObject5.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// termination
	glfwTerminate();
	return 0;
}