#include "Camera.h"

void cleanup() {
	glfwTerminate();
}

glm::vec4 randomColor() {
	return glm::vec4(
		(float)(rand() % 255) / 255,
		(float)(rand() % 255) / 255,
		(float)(rand() % 255) / 255,
		1.0
	);
}

int main(const int argc, const char* const argv[]) {
	try {
		if (!glfwInit()) {
			throw std::runtime_error("Error: failed to initialize glfw");
		}

		int width = 800;
		int height = 600;

		if (argc >= 3 && atoi(argv[1]) > 0 && atoi(argv[2]) > 0) {
			width = atoi(argv[1]);
			height = atoi(argv[2]);
		}

		cppogl::sWindow window = std::make_shared<cppogl::Window>(cppogl::Window(width, height, "cpp-opengl"));

		glewExperimental = true;
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			throw std::runtime_error("Error: failed to initialize glew: " + std::string((const char*)glewGetErrorString(err)));
		}
		
		cppogl::sShaderProgram basic3D = std::make_shared<cppogl::ShaderProgram>(cppogl::ShaderProgram("basic3D", "shader/basic3D.vert", "shader/basic3D.frag"));
		
		// ShaderManager will be used for shader lookup when materials are incorporated
		cppogl::ShaderManager shaderManager = {
			basic3D,
		};

		cppogl::NoClipCamera camera(cppogl::PERSPECTIVE_PROJECTION, window, basic3D);
		camera.translate(-0.1, 0.0, -0.5);
		
		srand(clock());
		cppogl::Triangle triangleA = cppogl::Triangle(
			basic3D,
			1.0,
			1.0,
			glm::radians(60.0f),
			{ randomColor(), randomColor(), randomColor() }
		);

		cppogl::Triangle triangleB = cppogl::Triangle(
			basic3D,
			1.0,
			1.0,
			glm::radians(60.0f),
			glm::vec4(0.0, 0.5, 0.75, 1.0)
		);
		triangleB.rotate(0.0, 0.0, glm::radians(60.0f));
		triangleB.translate(0.0, 0.0, 1.0);

		glClearColor(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		float deltaTime = 0.0f;
		clock_t currentTime = clock();
		clock_t previousTime = currentTime;
		while (!glfwWindowShouldClose(window->window)) {
			deltaTime = ((float)currentTime - (float)previousTime) / CLOCKS_PER_SEC;
			previousTime = currentTime;
			currentTime = clock();

			glViewport(0, 0, window->width(), window->height());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			basic3D->use();

			camera.update(deltaTime);

			triangleA.render();
			triangleB.render();

			checkGLErrors(__LINE__);
			
			glfwSwapBuffers(window->window);
			glfwPollEvents();
		}
	}
	catch (std::runtime_error &e) {
		std::cout << e.what();
		cleanup();
		std::cin.get();
		return -1;
	}
	cleanup();
	return 0;
}