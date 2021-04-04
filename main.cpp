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

#include <SOIL2.h>

#include "shader.hpp"
#include "camera.hpp"

using namespace cg;
namespace fs = std::filesystem;

// window settings
int screenWidth = 800;
int screenHeight = 600;

// input image files
const char* const HEIGHTMAP_FILE = "assets/heightmap.bmp";
const char* const TEXTURE_FILE = "assets/terrain-texture3.bmp";
const char* const DETAIL_FILE = "assets/detail.bmp";
const char* const SKYBOX_FILES[5] = {
	"assets/SkyBox/SkyBox0.bmp",
	"assets/SkyBox/SkyBox1.bmp",
	"assets/SkyBox/SkyBox2.bmp",
	"assets/SkyBox/SkyBox3.bmp",
	"assets/SkyBox/SkyBox4.bmp",
};
const char* const WATER_FILE = "assets/SkyBox/SkyBox5.bmp";

// -----------------------------------------------------------

int mapWidth = 0, mapHeight = 0, mapChannels = 0;
unsigned char* heightmap = nullptr;

GLuint skyboxTextures[5]{0};

// -----------------------------------------------------------

// normalized coordinates
constexpr GLfloat vertices[] = {
	// Positions        // Colors
	0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // Bottom Right
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // Bottom Left
	0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f   // Top 
};

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

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

	// ---------------------------------------------------------------

	/* load an image as a heightmap, forcing greyscale (so channels should be 1) */
	heightmap = SOIL_load_image(
		HEIGHTMAP_FILE,
		&mapWidth, &mapHeight, &mapChannels,
		SOIL_LOAD_L
	);

	if (!heightmap) {
		std::cerr << "Error loading heightmap '" << HEIGHTMAP_FILE << "'" << std::endl;
		glfwTerminate();
		return -3;
	}

	/* load skybox images as OpenGL texture */
	for (int i = 0; i < 5; i++) {
		skyboxTextures[i] = SOIL_load_OGL_texture(
			SKYBOX_FILES[i],
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

		/* check for an error during the load process */
		if (skyboxTextures[i] == 0) {
			std::cerr << "Error loading skybox image '" << SKYBOX_FILES[i] << "'" << std::endl;
			glfwTerminate();
			return -3;
		}
	}

	// -----------------------------------------

	// Install GLSL Shader programs
	auto shaderProgram = Shader::Create("VertexShader.vert", "FragmentShader.frag");
	if (shaderProgram == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

	// ---------------------------------------------------------------

	// Set up vertex data (and buffer(s)) and attribute pointers

	// bind VAO
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// bind VBO, buffer data to it
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// set vertex attribute pointers
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// unbind VBO & VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);


	

	// -----------------------------------------

	// Update loop

	while (glfwWindowShouldClose(window) == 0) {
		// check event queue
		glfwPollEvents();

		/* your update code here */
	
		// draw background
		GLfloat red = 0.2f;
		GLfloat green = 0.3f;
		GLfloat blue = 0.3f;
		glClearColor(red, green, blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw a triangle
		shaderProgram->Use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	/* done with the heightmap, free up the RAM */
	SOIL_free_image_data(heightmap);

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