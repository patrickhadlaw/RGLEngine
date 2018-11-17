// cpp-opengl.cpp

#include "Application.h"
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

const float UI_TICK = 20.0f / 1000.0f;

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

		cppogl::Application app = cppogl::Application("cppogl", window);

		app.initialize();

		cppogl::sShaderProgram basic3D = cppogl::sShaderProgram(new cppogl::ShaderProgram("basic3D", "shader/basic3D.vert", "shader/basic3D.frag"));
		app.addShader(basic3D);
		cppogl::sShaderProgram textured3D = cppogl::sShaderProgram(new cppogl::ShaderProgram("textured3D", "shader/textured3D.vert", "shader/textured3D.frag"));
		app.addShader(textured3D);
		cppogl::sShaderProgram text = cppogl::sShaderProgram(new cppogl::ShaderProgram("text", "shader/text.vert", "shader/text.frag"));
		app.addShader(text);

		cppogl::sFontFamily roboto = std::make_shared<cppogl::FontFamily>(cppogl::FontFamily("roboto", {
			{ cppogl::FontFamily::REGULAR, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Regular.ttf")) },
			{ cppogl::FontFamily::BOLD, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Bold.ttf")) },
			{ cppogl::FontFamily::ITALIC, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Italic.ttf")) },
			{ cppogl::FontFamily::ITALIC_BOLD, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-BoldItalic.ttf")) },
			{ cppogl::FontFamily::LIGHT, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Light.ttf")) },
			{ cppogl::FontFamily::ITALIC_LIGHT, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-LightItalic.ttf")) } }));

		app.addResource(roboto);

		std::shared_ptr<cppogl::NoClipCamera> camera = std::shared_ptr<cppogl::NoClipCamera>(new cppogl::NoClipCamera(cppogl::PERSPECTIVE_PROJECTION, window));
		camera->translate(-0.1, 0.0, -0.5);

		cppogl::sRenderableLayer mainLayer = cppogl::sRenderableLayer(new cppogl::RenderableLayer("main", camera));
		app.addLayer(mainLayer);

		cppogl::UI::sLayer uiLayer = cppogl::UI::sLayer(new cppogl::UI::Layer("ui", UI_TICK));
		app.addLayer(uiLayer);

		srand(clock());
		std::shared_ptr<cppogl::Triangle> triangleA = std::shared_ptr<cppogl::Triangle>(new cppogl::Triangle(
			app.getContext(),
			"basic3D",
			1.0,
			1.0,
			glm::radians(60.0f),
			{ randomColor(), randomColor(), randomColor() }
		));
		triangleA->id = "triangleA";
		mainLayer->addRenderable(triangleA);

		std::shared_ptr<cppogl::ImageRect> rect = std::shared_ptr<cppogl::ImageRect>(new cppogl::ImageRect(app.getContext(), "textured3D", 1.0, 1.0, "res/sky.png"));
		rect->translate(0.0, 2.0, 0.0);
		triangleA->id = "rect";
		mainLayer->addRenderable(rect);

		std::shared_ptr<cppogl::Triangle> triangleB = std::shared_ptr<cppogl::Triangle>(new cppogl::Triangle(
			app.getContext(),
			"basic3D",
			1.0,
			1.0,
			glm::radians(60.0f),
			glm::vec4(0.0, 0.5, 0.75, 1.0)
		));
		triangleB->id = "triangleB";
		mainLayer->addRenderable(triangleB);
		triangleB->rotate(0.0, 0.0, glm::radians(60.0f));
		triangleB->translate(0.0, 0.0, 1.0);

		cppogl::TextAttributes attrib{ cppogl::FontFamily::BOLD, cppogl::UnitValue::parse("16pt"), cppogl::UnitVector2D(300.0f, 0.0f, cppogl::Unit::PT), cppogl::UnitVector2D(0.0, 0.0) };
		std::shared_ptr<cppogl::Text> fpsText = std::make_shared<cppogl::Text>(cppogl::Text(app.getContext(), "text", "roboto", "Framerate: ", attrib));
		fpsText->id = "fpsText";
		uiLayer->addElement(fpsText);
		std::shared_ptr<cppogl::Text> wrapTest = std::make_shared<cppogl::Text>(cppogl::Text(app.getContext(),
			"text",
			"roboto",
			"Hello world! This is a test of word wrapping, will it wrap, mabye.",
			cppogl::TextAttributes{ cppogl::FontFamily::LIGHT, cppogl::UnitValue::parse("16pt"), cppogl::UnitVector2D(300.0f, 0.0f, cppogl::Unit::PT), cppogl::UnitVector2D(0.0, 0.0) }
		));
		wrapTest->id = "wrapTest";
		uiLayer->addElement(wrapTest);

		cppogl::UI::sLinearAligner textAligner = cppogl::UI::sLinearAligner(new cppogl::UI::LinearAligner({
			fpsText,
			wrapTest
		}));
		textAligner->id = "textAligner";
		uiLayer->addLogicNode(textAligner);

		int framerate = 0;

		while (!window->shouldClose()) {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			app.render();
			if (uiLayer->tick()) {
				fpsText->update(std::string("Framerate: ") + std::to_string(framerate));
			}
			framerate = (int)(1 / mainLayer->getFrameDelay());

			app.update();
		}
	}
	catch (cppogl::Exception& e) {
		cleanup();
		std::cin.get();
		return -1;
	}
	catch (std::exception& e) {
		cppogl::Exception except = cppogl::Exception(e.what(), EXCEPT_DETAIL_DEFAULT);
		std::cout << except.message();
		cleanup();
		std::cin.get();
		return -1;
	}
	catch (...) {
		cppogl::Exception except = cppogl::Exception("UNHANDLED EXCEPTION", EXCEPT_DETAIL_DEFAULT);
		std::cout << except.message();
		cleanup();
		std::cin.get();
		return -1;
	}
	cleanup();
	return 0;
}