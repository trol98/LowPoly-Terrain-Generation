#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "Camera.h"
#include "Color.h"
#include "NormalGenerator.h"
#include "IndexGenerator.h"

#include "vendor/noise/FastNoise.h"

#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


// SIZE is the overall size of the terrain so the terrain will be SIZE x SIZE
// VERTEX_COUNT is the number of vertiecies on one side of the terrain
// so the size of one square will be SIZE / VERTEX_COUNT
const unsigned int VERTEX_COUNT = 1000;
const float SIZE = 50.0f;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// F1 F2 change the rendering mode
unsigned char wireFrame;

glm::vec3 LIGHT_POS = glm::vec3(1.5f, 3.f, 1.5f);

//Color generation settings
constexpr float COLOUR_SPREAD = 0.45f; // 0.45
const std::vector<Color> TERRAIN_COLS = { Color(201, 178, 99),Color(135, 184, 82),Color(80, 171, 93), Color(120, 120, 120),Color(200, 200, 210) };

NormalGenerator normalGenerator(VERTEX_COUNT);

int storeQuad1(unsigned int* indices, int pointer, int topLeft, int topRight, int bottomLeft, int bottomRight,
	bool mixed);

int storeQuad2(unsigned int* indices, int pointer, int topLeft, int topRight, int bottomLeft, int bottomRight,
	bool mixed);




int main()
{
	std::srand(time(nullptr));
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", /*glfwGetPrimaryMonitor()*/NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSwapInterval(1);



	// glew: load all OpenGL function pointers
	// ---------------------------------------
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	Shader ourShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
	ColourGenerator colorGen(TERRAIN_COLS, COLOUR_SPREAD);
	IndexGenerator indexGenerator;

	FastNoise noiseGenerator(std::rand());
	noiseGenerator.SetFrequency(0.06f);
	noiseGenerator.SetFractalOctaves(8);
	noiseGenerator.SetInterp(FastNoise::Interp::Linear);
	noiseGenerator.SetGradientPerturbAmp(3.0f);

	//allocationg memory
	int count = VERTEX_COUNT * VERTEX_COUNT;
	float* vertices = new float[count * 3];
	float* normals = new float[count * 3];
	float* colors = new float[count * 3];

	unsigned int* indices = new unsigned int[6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT - 1)];

	float** heights = new float* [VERTEX_COUNT];
	for (int i = 0; i < VERTEX_COUNT; i++)
		heights[i] = new float[VERTEX_COUNT];


	//genereting terrain values
	int vertexPointer = 0;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {
			// vertices
			vertices[vertexPointer * 3 + 0] = (float)j / ((float)VERTEX_COUNT - 1) * SIZE;

			float noiseValue = noiseGenerator.GetPerlinFractal(j, i);
			vertices[vertexPointer * 3 + 1] = noiseValue;
			heights[j][i] = noiseValue;

			vertices[vertexPointer * 3 + 2] = (float)i / ((float)VERTEX_COUNT - 1) * SIZE;

			vertexPointer++;
		}
	}

	float low = 2.0f;
	float high = -2.0f;
	for (int i = 0; i < VERTEX_COUNT; i++)
	{
		for (int j = 0; j < VERTEX_COUNT; j++)
		{
			if (heights[j][i] < low)
				low = heights[j][i];
			if (heights[j][i] > high)
				high = heights[j][i];
		}
	}
	std::cout << "Heighest: " << high << std::endl;
	std::cout << "Lowest: " << low << std::endl;

	int pointer = 0;
	for (int col = 0; col < VERTEX_COUNT - 1; col++)
	{
		for (int row = 0; row < VERTEX_COUNT - 1; row++)
		{
			int topLeft = (row * VERTEX_COUNT) + col;
			int topRight = topLeft + 1;
			int bottomLeft = ((row + 1) * VERTEX_COUNT) + col;
			int bottomRight = bottomLeft + 1;
			if (row % 2 == 0)
			{
				pointer = indexGenerator.storeQuad1(indices, pointer, topLeft, topRight, bottomLeft, bottomRight, col % 2 == 0);
			}
			else
			{
				pointer = indexGenerator.storeQuad2(indices, pointer, topLeft, topRight, bottomLeft, bottomRight, col % 2 == 0);
			}
		}
	}
	normalGenerator.generateNormals(heights, normals);
	colorGen.generateColours(heights, 1.0f, colors, VERTEX_COUNT);

	unsigned int verticesVBO, normalsVBO, colorsVBO, EBO, VAO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &verticesVBO);
	glGenBuffers(1, &normalsVBO);
	glGenBuffers(1, &colorsVBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), normals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
	glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), colors, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT - 1) * sizeof(unsigned int), indices, GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);

	delete[] vertices;
	delete[] normals;
	delete[] indices;
	delete[] colors;

	for (int i = 0; i < VERTEX_COUNT; i++)
		delete[] heights[i];
	delete[] heights;

	// configure global opengl state
	// -----------------------------

	glEnable(GL_DEPTH_TEST);
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

	ourShader.use();
	ourShader.setVec3("lightPos", LIGHT_POS);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, 6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT - 1), GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &verticesVBO);
	glDeleteBuffers(1, &normalsVBO);
	glDeleteBuffers(1, &colorsVBO);
	glDeleteBuffers(1, &EBO);


	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//std::cout << camera.Position.x << ' ' << camera.Position.y << ' ' << camera.Position.z << std::endl;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
