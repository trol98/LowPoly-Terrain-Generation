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

#include <ctime>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, double deltaTime);

constexpr unsigned SCR_WIDTH = 1920;
constexpr unsigned SCR_HEIGHT = 1440;

// camera
Camera camera(glm::vec3(-3.0f, 3.0f, 5.0f));
double lastX = SCR_WIDTH / 2.0;
double lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// settings
// use F1, F2, F3
bool flag = false; 

// triangles size = SIZE / VERTEX_COUNT
constexpr unsigned int VERTEX_COUNT = 200;
constexpr float SIZE = 10.0f;


//Color generation settings
constexpr float COLOUR_SPREAD = 0.45f; 
const std::vector<Color> TERRAIN_COLS = {
	Color(201, 178,  99),
	Color(135, 184,  82),
	Color( 80, 171,  93),
	Color(120, 120, 120),
	Color(200, 200, 210)};

constexpr FastNoise::NoiseType	 NOISE_TYPE = FastNoise::NoiseType::ValueFractal;
constexpr float					 FREQUENCY = 0.07f;
constexpr unsigned				 FRACTAL_OCTAVES = 8;
constexpr FastNoise::Interp	     INTERP = FastNoise::Interp::Linear;
constexpr FastNoise::FractalType TYPE = FastNoise::FractalType::FBM;
constexpr float					 LACUNARITY = 2.5f;
constexpr float					 GAIN = 0.5f;

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", /*glfwGetPrimaryMonitor()*/nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);



	// glew: load all OpenGL function pointers
	// ---------------------------------------
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}



	Shader ourShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
	NormalGenerator normalGenerator(VERTEX_COUNT);
	ColourGenerator colorGen(TERRAIN_COLS, COLOUR_SPREAD, VERTEX_COUNT);
	IndexGenerator indexGenerator;

	FastNoise noiseGenerator(std::rand());
	noiseGenerator.SetNoiseType(NOISE_TYPE);
	noiseGenerator.SetFrequency(FREQUENCY);
	noiseGenerator.SetFractalOctaves(FRACTAL_OCTAVES);
	noiseGenerator.SetInterp(INTERP);
	noiseGenerator.SetFractalType(TYPE);
	noiseGenerator.SetFractalLacunarity(LACUNARITY);
	noiseGenerator.SetFractalGain(GAIN);


	//allocationg memory
	int count = VERTEX_COUNT * VERTEX_COUNT;
	float* vertices = new float[count * 3];
	float* normals = new float[count * 3];
	float* colors = new float[count * 3];

	float** heights = new float* [VERTEX_COUNT];
	for (int i = 0; i < VERTEX_COUNT; i++)
		heights[i] = new float[VERTEX_COUNT];

	unsigned int* indices = new unsigned int[6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT - 1)];



	//genereting terrain values
	int vertexPointer = 0;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {
			// vertices
			vertices[vertexPointer * 3 + 0] = (float)j / ((float)VERTEX_COUNT - 1) * SIZE;
			vertices[vertexPointer * 3 + 2] = (float)i / ((float)VERTEX_COUNT - 1) * SIZE;

			vertexPointer++;
		}
	}

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

	int heightPointer = 0;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {

			float noiseValue = noiseGenerator.GetNoise(j, i);
			vertices[heightPointer * 3 + 1] = noiseValue;
			heights[j][i] = noiseValue;

			heightPointer++;
		}
	}

	normalGenerator.generateNormals(heights, normals);
	colorGen.generateColours(heights, 1.0f, colors);

	// configure global opengl state
	// -----------------------------

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


	glEnable(GL_DEPTH_TEST);
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

	ourShader.use();

	// timing
	double deltaTime = 0.0f;
	double lastFrame = 0.0f;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		flag = false;

		// input
		// -----
		processInput(window, deltaTime);

		// if true we need to update our draw data
		if (flag) 
		{
			noiseGenerator.SetSeed(static_cast<int>(time(nullptr)));
			int heightPointer = 0;
			for (int i = 0; i < VERTEX_COUNT; i++) {
				for (int j = 0; j < VERTEX_COUNT; j++) {

					float noiseValue = noiseGenerator.GetNoise(j, i);
					vertices[heightPointer * 3 + 1] = noiseValue;
					heights[j][i] = noiseValue;

					heightPointer++;
				}
			}

			normalGenerator.generateNormals(heights, normals);
			colorGen.generateColours(heights, 1.0f, colors);



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
		}

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


	delete[] vertices;
	delete[] normals;
	delete[] indices;
	delete[] colors;

	for (int i = 0; i < VERTEX_COUNT; i++)
		delete[] heights[i];
	delete[] heights;

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
void processInput(GLFWwindow* window, double deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
		flag = true;
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
		lastX =	xpos;
		lastY =	ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
