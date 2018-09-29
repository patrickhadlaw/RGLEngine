#include "Camera.h"
#include "Font.h"

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

const float UI_TICK = 100.0f / 1000.0f;

int main(const int argc, const char* const argv[]) {
	try {
		if (!glfwInit()) {
			throw cppogl::Exception("failed to initialize glfw", EXCEPT_DETAIL_DEFAULT);
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
			throw cppogl::Exception("Error: failed to initialize glew: " + std::string((const char*)glewGetErrorString(err)), EXCEPT_DETAIL_DEFAULT);
		}
		
		cppogl::sShaderProgram basic3D = std::make_shared<cppogl::ShaderProgram>(cppogl::ShaderProgram("basic3D", "shader/basic3D.vert", "shader/basic3D.frag"));
		cppogl::sShaderProgram textured3D = std::make_shared<cppogl::ShaderProgram>(cppogl::ShaderProgram("textured3D", "shader/textured3D.vert", "shader/textured3D.frag"));
		cppogl::sShaderProgram text = std::make_shared<cppogl::ShaderProgram>(cppogl::ShaderProgram("text", "shader/text.vert", "shader/text.frag"));

		cppogl::sFont consolas = std::make_shared<cppogl::Font>(cppogl::Font(window, "res/Consolas.ttf"));

		// ShaderManager will be used for shader lookup when materials are incorporated
		cppogl::ShaderManager shaderManager = {
			basic3D,
			textured3D,
			text
		};

		cppogl::NoClipCamera camera(cppogl::PERSPECTIVE_PROJECTION, window);
		camera.translate(-0.1, 0.0, -0.5);
		
		srand(clock());
		cppogl::Triangle triangleA = cppogl::Triangle(
			basic3D,
			1.0,
			1.0,
			glm::radians(60.0f),
			{ randomColor(), randomColor(), randomColor() }
		);

		cppogl::ImageRect rect = cppogl::ImageRect(textured3D, 1.0, 1.0, "res/sky.png");
		rect.translate(0.0, 2.0, 0.0);

		cppogl::Triangle triangleB = cppogl::Triangle(
			basic3D,
			1.0,
			1.0,
			glm::radians(60.0f),
			glm::vec4(0.0, 0.5, 0.75, 1.0)
		);
		triangleB.rotate(0.0, 0.0, glm::radians(60.0f));
		triangleB.translate(0.0, 0.0, 1.0);

		cppogl::Text fpsText = cppogl::Text(window, text, consolas, "Framerate: ", 16);

		glClearColor(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float deltaTime = 0.0f;
		clock_t currentTime = clock();
		clock_t previousTime = currentTime;

		float uiTime = 0.0f;
		int framerate = 0;

		while (!window->shouldClose()) {
			previousTime = currentTime;
			currentTime = clock();
			deltaTime = ((float)currentTime - (float)previousTime) / CLOCKS_PER_SEC;
			uiTime += deltaTime;
			framerate = (int)(1 / deltaTime);

			glViewport(0, 0, window->width(), window->height());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			basic3D->use();

			camera.update(deltaTime);

			camera.bind(basic3D);

			triangleA.render();
			triangleB.render();

			textured3D->use();

			camera.bind(textured3D);

			rect.render();

			text->use();
			if (uiTime > UI_TICK) {
				fpsText.update(std::string("Framerate: ") + std::to_string(framerate));
				uiTime = 0.0f;
			}
			fpsText.render();

			checkGLErrors(__LINE__);
			
			window->update();
		}
	}
	catch (cppogl::Exception &e) {
		std::cout << e.log();
		cleanup();
		std::cin.get();
		return -1;
	}
	catch (std::exception &e) {
		std::cout << "UNHANDLED EXCEPTION: " << e.what();
		cleanup();
		std::cin.get();
		return -1;
	}
	cleanup();
	return 0;
}