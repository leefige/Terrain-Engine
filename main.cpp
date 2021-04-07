/*
 * OpenGL version 4.6 project.
 */
#include <ctime>
#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

#include "shader.hpp"
#include "camera.hpp"
#include "terrain_engine.h"

namespace fs = std::filesystem;
using namespace cg;

// input image files
constexpr auto HEIGHTMAP_FILE = "assets/heightmap.bmp";
constexpr auto TEXTURE_FILE = "assets/terrain-texture3.bmp";
constexpr auto DETAIL_FILE = "assets/detail.bmp";
constexpr const char* SKYBOX_FILES[5] = {
	"assets/SkyBox/SkyBox0.bmp",
	"assets/SkyBox/SkyBox1.bmp",
	"assets/SkyBox/SkyBox2.bmp",
	"assets/SkyBox/SkyBox3.bmp",
	"assets/SkyBox/SkyBox4.bmp",
};
constexpr auto WATER_FILE = "assets/SkyBox/SkyBox5.bmp";

constexpr auto SKYBOX_VERT_SHADER = "shaders/skybox.vert";
constexpr auto SKYBOX_FRAG_SHADER = "shaders/skybox.frag";
constexpr auto WATER_VERT_SHADER = "shaders/water.vert";
constexpr auto WATER_FRAG_SHADER = "shaders/water.frag";
constexpr auto TERRAIN_VERT_SHADER = "shaders/terrain.vert";
constexpr auto TERRAIN_FRAG_SHADER = "shaders/terrain.frag";

// --------------------------------------

// window settings
int screenWidth = 1902;
int screenHeight = 1080;
bool keys[1024]{false};

Camera camera(glm::vec3(0.0f, 1.5f, 15.0f), glm::vec3(0.0f, 0.0f, -1.0f), 2.0f);

// -----------------------------------------------------------

// helper functions
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void moveCamera(GLfloat deltaTime);
void saveScreenshot();

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Yifei Li - Terrain Engine", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Error creating window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// use newly created window as context
	glfwMakeContextCurrent(window);

	// register callbacks
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);

	// ---------------------------------------------------------------

	// Connect GLAD to GLFW by registerring glfwGetProcAddress() as GLAD loader function,
	// this must be done after setting current context

	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cerr << "Error registerring gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return -2;
	}

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// ---------------------------------------------------------------

	// Load terrain engine resources
	TerrainEngine engine;

	/* load an image as a heightmap, forcing greyscale (so channels should be 1) */
	if (!engine.LoadHeightmap(HEIGHTMAP_FILE)) {
		std::cerr << "Error loading heightmap '" << HEIGHTMAP_FILE << "'" << std::endl;
		glfwTerminate();
		return -3;
	}

	if (!engine.LoadTerrainTexture(TEXTURE_FILE, DETAIL_FILE)) {
		std::cerr << "Error loading land texture '" << TEXTURE_FILE << "'" << std::endl;
		glfwTerminate();
		return -3;
	}

	if (!engine.LoadWaterTexture(WATER_FILE)) {
		std::cerr << "Error loading water texture '" << WATER_FILE << "'" << std::endl;
		glfwTerminate();
		return -3;
	}

	if (!engine.LoadSkybox(SKYBOX_FILES)) {
		std::cerr << "Error loading skybox images" << std::endl;
		glfwTerminate();
		return -3;
	}

	// -----------------------------------------

	// Install GLSL Shader programs

	if (!engine.InstallSkyboxShaders(SKYBOX_VERT_SHADER, SKYBOX_FRAG_SHADER)) {
		std::cerr << "Error creating Shader Program for skybox" << std::endl;
		glfwTerminate();
		return -4;
	}

	if (!engine.InstallWaterShaders(WATER_VERT_SHADER, WATER_FRAG_SHADER)) {
		std::cerr << "Error creating Shader Program for water" << std::endl;
		glfwTerminate();
		return -4;
	}

	if (!engine.InstallTerrainShaders(TERRAIN_VERT_SHADER, TERRAIN_FRAG_SHADER)) {
		std::cerr << "Error creating Shader Program for terrain" << std::endl;
		glfwTerminate();
		return -4;
	}

	// -----------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Update loop
	GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
	GLfloat lastFrame = 0.0f;    // Time of last frame

	while (glfwWindowShouldClose(window) == 0) {
		// Calculate deltatime of current frame
		GLfloat currentFrame = GLfloat(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// check event queue
		glfwPollEvents();

		/* your update code here */
		moveCamera(deltaTime);
	
		// draw background
		GLfloat red = 0.2f;
		GLfloat green = 0.3f;
		GLfloat blue = 0.3f;
		glClearColor(red, green, blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Camera/View transformation
		glm::mat4 view = camera.ViewMatrix();

		// Projection
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom()), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

		// draw sky & water
		engine.DrawSkybox(view, projection);
		engine.DrawTerrain(view, projection);
		engine.DrawWater(view, projection, deltaTime);

		// swap buffer
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

/* ======================== helper functions ======================== */


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// exit when pressing ESC
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_PRINT_SCREEN && action == GLFW_PRESS) {
		saveScreenshot();
	}
	else if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			keys[key] = true;
		} else if (action == GLFW_RELEASE) {
			keys[key] = false;
		}
	}
}

void moveCamera(GLfloat deltaTime)
{
	// Camera controls
	if (keys[GLFW_KEY_W]) {
		camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
	}
	if (keys[GLFW_KEY_S]) {
		camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
	}
	if (keys[GLFW_KEY_A]) {
		camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
	}
	if (keys[GLFW_KEY_D]) {
		camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	static GLfloat lastX = GLfloat(screenWidth) / 2;
	static GLfloat lastY = GLfloat(screenHeight) / 2;

	GLfloat xoffset = GLfloat(xpos) - lastX;
	GLfloat yoffset = lastY - GLfloat(ypos); // Reversed since y-coordinates go from bottom to left
	lastX = GLfloat(xpos);
	lastY = GLfloat(ypos);

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(GLfloat(yoffset));
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	screenWidth = width;
	screenHeight= height;
	// resize window
	glViewport(0, 0, screenWidth, screenHeight);
}

void saveScreenshot()
{
	auto dir = fs::current_path() / "screenshots";
	if (!(fs::exists(dir) && fs::is_directory(dir))) {
		if (!fs::create_directories(dir)) {
			std::cerr << "Cannot create screenshot directory '" << dir << "'" << std::endl;
			return;
		}
	}

	std::time_t t = std::time(nullptr);
#ifdef __STDC_LIB_EXT1__
	struct tm buf;
	localtime_s(&t, &buf);
#else
	struct tm buf;
	localtime_s(&buf, &t);
#endif
	char mbstr[32];
	std::strftime(mbstr, sizeof(mbstr), "%Y%m%d-%H%M%S", &buf);
	auto filename = dir / (std::string("Terrain_Engine-") + mbstr + ".png");
	auto filenameStr = filename.string();

	if (SOIL_save_screenshot(filenameStr.c_str(), SOIL_SAVE_TYPE_PNG, 0, 0, screenWidth, screenHeight) == 0) {
		std::cerr << "Saving screenshot '" << filename <<"' failed" << std::endl;
	} else {
		std::cout << "Screenshot saved to '" << filenameStr << "'" << std::endl;
	}
}
