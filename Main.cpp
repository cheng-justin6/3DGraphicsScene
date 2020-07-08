// ===================================================================================
//
// Main.cpp  
//
// -----------------------------------
//
// By Justin Cheng
// Instancing and Bloom Demo
//
// ===================================================================================

// Standard Includes
#include "stdafx.h"
#include "cmath"
#include <math.h>
#include <time.h> 

// Include various libraries
#include "GL\glew.h"	// GLEW
#include "GL\glfw3.h"	// GLFW

#include "glm\glm.hpp"	// GLM stuff
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
 
// Custom header includes
#include "UseShader.h"
#include "ModelObj.h"
#include "Camera.h"

// Imgui test
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

using namespace std;
using namespace glm;

#define PI 3.1415926535897932384626433832795
#define POINT_LIGHTS 2

// Function Prototypes
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void doMovement();
GLfloat distToLinear(GLfloat dist);
GLfloat distToQuad(GLfloat dist);
void RenderScene(Shader &shader);
void RenderFX(Shader &shader);
void RenderQuad();

// Window Size
const GLuint SCREEN_WIDTH = 1280;
const GLuint SCREEN_HEIGHT = 720;

// Camera Settings
Camera camera(vec3(0.0f, 2.5f, 8.0f));
GLfloat lastX = SCREEN_WIDTH / 2.0;
GLfloat lastY = SCREEN_HEIGHT / 2.0;
bool keys[1024];
bool keysPressed[1024];
bool firstMouse = true;
bool camRotate = true;

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Light Settings
vec3 lightPos[POINT_LIGHTS];
GLboolean hdr = true; 
GLboolean bloom = true;
GLfloat exposure = 3.0f; 

// Imgui
void drawGui();
bool free_look = true;

// Misc
Model figureModel, groundModel, poiModel, particleModel;
const GLint instanceNum = 10000;
mat4 instanceMatrices[instanceNum];

// Framebuffer Texture
GLuint quadVAO = 0;
GLuint quadVBO;
GLuint colorBuffer[2]; 
GLuint hdrBuffer; 
GLuint ppBuffer[2];
GLuint ppColorBuffer[2];

// Main Function
int main(int argc, char **argv) {

	cout << "Starting GLFW context, OpenGL 3.3" << endl;

	cout << "-----------------------------------\n" 
		<< " Demo Scene\n"
		<< "-----------------------------------\n"
		<< "Features Implemented:\n"
		<< "* Framebuffers: High Dynamic Range\n"
		<< "* Framebuffers: Bloom\n"
		<< "* Instancing: 10000 objects\n"
		<< endl

		<< " Camera Controls:\n"
		<< "* Move around the scene with [W][A][S][D] keys\n"
		<< "* Adjust camera angle by moving the [MOUSE]\n"
		<< "* Zoom in and out using the [SCROLL-WHEEL]\n"
		<< "* Press [R] for camera auto-rotation\n"
		<< "* Use [SPACE] to lock mouse controls for camera\n"
		<< "* Exit the program with the [ESC] key\n"
		<< endl

		<< " Feature Controls:\n"
		<< "* Use [H] to toggle HDR on/off \n"
		<< "* Use [B] to toggle Bloom on/off \n"
		<< "* Use [Q] & [E] to increase/decrease light exposure \n"	
		<< endl;

	// Initialzie required options -----------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Make a Window  & Set Callbacks --------------------
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Demo Scene", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	glewInit();

	// Define viewport dimensions 
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Enable Depth Test --------------------------------
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	// Build, Compile, and Link Shaders -----------------
	Shader shader("Shaders/main_vshader.glsl", "Shaders/main_fshader.glsl");
	Shader blurShader("Shaders/blur_vshader.glsl", "Shaders/blur_fshader.glsl");
	Shader bloomShader("Shaders/bloom_vshader.glsl", "Shaders/bloom_fshader.glsl");

	bloomShader.Use();
	glUniform1i(glGetUniformLocation(bloomShader.Program, "scene"), 0);
	glUniform1i(glGetUniformLocation(bloomShader.Program, "bloomTex"), 1);

	// Load Models --------------------------------------

	figureModel = Model("Models/Objs/Char.obj");
	poiModel = Model("Models/Objs/FirePoi.obj");
	groundModel = Model("Models/Objs/Ground.obj");
	particleModel = Model("Models/Objs/Butterfly2.obj");

	// Light positions
	lightPos[0] = vec3(-1.6f, 0.5f, 0.55f);
	lightPos[1] = vec3(1.6f, 4.6f, 1.55f);

	// Set up instancing here ---------------------------
	
	// Generate orientation of each butterfly
	srand(glfwGetTime());
	GLfloat radius = 10.00f;
	GLfloat offset = 2.50f;
	GLfloat expanse = 2000.0f;
	for (GLuint i = 0; i < instanceNum; i++) {
		mat4 model = mat4();
		model = translate(model, vec3(0.0f, 0.0f, 0.0f));

		GLfloat x = (expanse / 2 - expanse * ((rand() % 100) / 100.f));
		GLfloat y = 0.32 * (expanse * ((rand() % 100) / 100.0));
		GLfloat z = (expanse / 2 - expanse * ((rand() % 100) / 100.0));
		model = translate(model, vec3(x, y, z));

		GLfloat scale_size = 0.5 + 0.5 * ((rand() % 100) / 100.0);
		model = scale(model, vec3(scale_size));
		
		GLfloat rotation_x = 5 - (rand() % 10);
		GLfloat rotation_y = atan2(z, x);
		GLfloat rotation_z = 5 - (rand() % 10);
		model = rotate(model, rotation_x, vec3(1.0, 0.0, 0.0));
		model = rotate(model, rotation_y, vec3(0.0, 1.0, 0.0));
		model = rotate(model, rotation_z, vec3(0.0, 0.0, 1.0));

		instanceMatrices[i] = model;
	}

	// Attach orientation to butterfly VAO
	for (GLuint i = 0; i < particleModel.meshes.size(); i++) {
		GLuint VAO = particleModel.meshes[i].VAO;
		GLuint buffer;
		glBindVertexArray(VAO);
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, instanceNum * sizeof(glm::mat4), &instanceMatrices[0], GL_STATIC_DRAW);

		for (int i = 0; i < 4; i++) {
			glEnableVertexAttribArray(3 + i);
			glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)*i));		
			glVertexAttribDivisor(3 + i, 1);
		}
		glBindVertexArray(0);
	}

	// Initialize HDR / Bloom ---------------------------
	glGenFramebuffers(1, &hdrBuffer); 
	glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);

	// Floating point color buffer: 1 for rendering, 1 for brightness
	glGenTextures(2, colorBuffer);
	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], 0);
	}

	// Depth buffer
	GLuint depthRenderbuffer;
	glGenRenderbuffers(1, &depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

	// Assign buffers to color attachments
	static GLuint colorAttach[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, colorAttach);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clear colorbuffer
	//glClearColor(0.1f, 0.05f, 0.15f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Initialize Ping-Pong Buffers -----------------
	// The Ping-Pong buffers are used so you can Gaussian blur a framebuffer multiple times	
	glGenFramebuffers(2, ppBuffer);
	glGenTextures(2, ppColorBuffer);
	for (GLuint i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, ppBuffer[i]);
		glBindTexture(GL_TEXTURE_2D, ppColorBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ppColorBuffer[i], 0);
	}

	// Imgui Test
	ImGui_ImplGlfwGL3_Init(window, false);

	// Loop ---------------------------------------------
	while (!glfwWindowShouldClose(window)) {
		// Calculate deltatime between frames
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check for events 
		glfwPollEvents();
		doMovement();

		// Imgui 
		ImGui_ImplGlfwGL3_NewFrame();
		drawGui();

		// Set up camera --------------------------
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Use();
		mat4 view;

		if (camRotate) {
			camera.position.x = sin(0.3*glfwGetTime()) * 9.5f;
			camera.position.z = cos(0.3*glfwGetTime()) * 9.5f;
			view = glm::lookAt(glm::vec3(camera.position.x, camera.position.y, camera.position.z), glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else 
			view = camera.GetViewMatrix();

		mat4 projection = perspective(camera.zoom, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, value_ptr(view));

		// Set light uniforms ---------------------
		vec3 lightColor;

		GLfloat lightDist = sin(glfwGetTime()) * 9.0f;
		GLfloat linear = distToLinear(29 + lightDist);
		GLfloat quadratic = distToQuad(29 + lightDist);

		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].lightColor"), 0.45f, 0.3f, 0.3f);  
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].lightPos"), lightPos[0].x,lightPos[0].y,lightPos[0].z);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), linear);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), quadratic);

		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].lightColor"), 0.45f, 0.3f, 0.3f);  
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].lightPos"), lightPos[1].x, lightPos[1].y, lightPos[1].z);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), linear);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), quadratic);

		glUniform3fv(glGetUniformLocation(shader.Program, "viewPos"), 1, &camera.position[0]);

		// Pass1: Render scene into framebuffer 
		// --------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Use();
		RenderScene(shader);	
		RenderFX(shader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Blur the bright areas of the framebuffer using pingpong and gaussian blur
		GLboolean horiz = true;
		GLboolean first_blur = true;
		blurShader.Use();
		int iteration = 50;
		for (int i = 0; i < iteration; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, ppBuffer[horiz]);
			glUniform1i(glGetUniformLocation(blurShader.Program, "horizontal"), horiz);
			glBindTexture(GL_TEXTURE_2D, first_blur? colorBuffer[1] : ppColorBuffer[!horiz]);  // bind texture of other framebuffer (or scene if first iteration)
			RenderQuad();
			horiz = !horiz;
			if (first_blur)
				first_blur = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Pass2: Add HDR / Bloom effects to framebuffer 
		// --------------------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		bloomShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffer[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ppColorBuffer[!horiz]);
		glUniform1i(glGetUniformLocation(bloomShader.Program, "hdr"), hdr);
		glUniform1i(glGetUniformLocation(bloomShader.Program, "bloom"), bloom);
		glUniform1f(glGetUniformLocation(bloomShader.Program, "exposure"), exposure);
		RenderQuad();

		// Swap frame buffers
		ImGui::Render();
		glfwSwapBuffers(window);
	}

	// End ----------------------------------------------
	// Terminate
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}

//Imgui stuff
void drawGui() {
	bool show_test_window = true;
	bool show_another_window = false;
	//ImVec4 clear_color = ImColor(114, 144, 154);

	ImGui::Text("Framebuffers: High Dynamic Range (HDR), Bloom");
	ImGui::Text("Instancing: 10000 objects");
	ImGui::Text("\n");

	ImGui::Text("[W][A][S][D] - Pan camera | [ESC] - Exit program");
	ImGui::Text("[MOUSE SCROLL] - Look around | [MOUSE WHEEL] - Dolly camera");
	ImGui::Text("[R] - Auto-rotate camera | [SPACE] - Lock mouse");
	ImGui::Text("[H] - toggle HDR on/off | [B] - toggle Bloom on/off");
	ImGui::Text("[Q][E] - increase/decrease camera light exposure");
	ImGui::Text("\n");

	ImGui::Text("Auto-rotate: %s | Freelook: %s", camRotate ? "on" : "off", free_look ? "on" : "off");
	ImGui::Text("HDR: %s | Bloom: %s", hdr ? "on" : "off", bloom ? "on" : "off");
	ImGui::Text("Exposure: %f", exposure);
	ImGui::Text("\n");
	
	ImGui::Text("Bloom Texture Preview (<- Extracted | Blurred ->):");
	ImGui::ImageButton((void*)colorBuffer[1], ImVec2(192, 108));
	ImGui::SameLine();
	ImGui::ImageButton((void*)ppColorBuffer[1], ImVec2(192, 108));
}

// Display Models
void RenderScene(Shader &shader) {
	// Set Emission intensity;
	GLfloat emiInten;

	// Figure
	mat4 model;
	model = mat4();
	model = translate(model, vec3(0.0f, 0.0f, 0.0f));
	model = scale(model, vec3(0.2f, 0.2f, 0.2f));
	emiInten = sin(1.6 * glfwGetTime()) * 0.1f;
	glUniform1i(glGetUniformLocation(shader.Program, "instance"), 0);
	glUniform1f(glGetUniformLocation(shader.Program, "emiIntensity"), 0.9f + emiInten);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, value_ptr(model));
	figureModel.Draw(shader);

	// Flames
	model = mat4();
	model = translate(model, vec3(0.0f, 0.0f, 0.0f));
	model = scale(model, vec3(0.2f, 0.2f, 0.2f));
	emiInten = sin(glfwGetTime()) * 0.4f;
	glUniform1i(glGetUniformLocation(shader.Program, "instance"), 0);
	glUniform1f(glGetUniformLocation(shader.Program, "emiIntensity"), 0.6f + emiInten);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, value_ptr(model));
	poiModel.Draw(shader);	

	// Ground
	model = mat4();
	model = translate(model, vec3(0.0f, 0.0f, 0.0f));
	model = scale(model, vec3(0.2f, 0.2f, 0.2f));
	glUniform1i(glGetUniformLocation(shader.Program, "instance"), 0);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, value_ptr(model));
	groundModel.Draw(shader);	
}

// Display more FX stuff
void RenderFX(Shader &shader) {
	// Set Emission intensity;
	GLfloat partInten, partInten2, partInten3, partInten4;
	
	mat4 model;
	model = mat4();
	model = translate(model, vec3(0.0f, 0.0f, 0.0f));
	model = scale(model, vec3(0.025f, 0.025f, 0.025f));

	// Set timing for butterfly glows
	partInten = sin(0.5 * glfwGetTime()) * 0.3f;
	partInten2 = sin(0.5 * glfwGetTime() + 0.5 * PI) * 0.3f;
	partInten3 = sin(0.5 * glfwGetTime() + 1.0 * PI) * 0.3f;
	partInten4 = sin(0.5 * glfwGetTime() + 1.5 * PI) * 0.3f;
	glUniform1f(glGetUniformLocation(shader.Program, "particleIntensity1"), 0.7f + partInten);
	glUniform1f(glGetUniformLocation(shader.Program, "particleIntensity2"), 0.7f + partInten2);
	glUniform1f(glGetUniformLocation(shader.Program, "particleIntensity3"), 0.7f + partInten3);
	glUniform1f(glGetUniformLocation(shader.Program, "particleIntensity4"), 0.7f + partInten4);

	// Render butterfly as instance
	glUniform1i(glGetUniformLocation(shader.Program, "instance"), 1);
	glUniform1i(glGetUniformLocation(shader.Program, "instanceNum"), instanceNum);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, value_ptr(model));
	particleModel.DrawInstance(shader, instanceNum);
}

// Display framebuffer quad
void RenderQuad() {
	if (quadVAO == 0) {
		GLfloat quadVertices[] = {
			// Positions			// Texture Coords
			-1.0f, 1.0f, 0.0f,		0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,		1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		};

		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// Convert distance to light linear constant
GLfloat distToLinear(GLfloat dist) {
	return 5.4399f * pow(dist, -1.063);
}

// Convert distance to light quadratic constant
GLfloat distToQuad(GLfloat dist) {
	return 107.35f * pow(dist, -2.115);
}

// Track mouse movements
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	if (free_look) 
		camera.ProcessMouseMovement(xoffset, yoffset);	
}

// Track mouse scrollwheel
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

// Handler for whenever a key is pressed
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if(key >= 0 && key < 1024) {
		if(action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE) {
			keys[key] = false;
			keysPressed[key] = false;
		}
	}
}

// Track keyboard presses
void doMovement() {
	//ImGui_ImplGlfwGL3_KeyCallback();

	// Camera Movement
	GLfloat cameraSpeed = 8.0f * deltaTime;
	if(keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if(keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if(keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if(keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// HDR / Bloom Controls
	if (keys[GLFW_KEY_H] && !keysPressed[GLFW_KEY_H]) {
		hdr = !hdr;
		keysPressed[GLFW_KEY_H] = true;
	}

	if (keys[GLFW_KEY_B] && !keysPressed[GLFW_KEY_B]) {
		bloom = !bloom;
		keysPressed[GLFW_KEY_B] = true;
	}

	if (keys[GLFW_KEY_Q])
		exposure -= 2.0 * deltaTime;
	else if (keys[GLFW_KEY_E])
		exposure += 2.0 * deltaTime;

	// Freelook
	if (keys[GLFW_KEY_SPACE] && !keysPressed[GLFW_KEY_SPACE]) {
		free_look = !free_look;
		keysPressed[GLFW_KEY_SPACE] = true;
	}

	// Camera Auto-rotate
	if (keys[GLFW_KEY_R] && !keysPressed[GLFW_KEY_R]) {
		camRotate = !camRotate;
		keysPressed[GLFW_KEY_R] = true;
	}
}

