// cpp-opengl.cpp

#include "cpp-opengl.h"

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

template<size_t N, typename Type>
class Stack {
public:
	Stack() : Stack(Type{}) {

	}
	Stack(Type fill) {
		for (int i = 0; i < N; i++) {
			_stack[i] = fill;
		}
	}
	virtual ~Stack() {

	}

	void push(Type value) {
		for (int i = 0; i < N - 1; i++) {
			_stack[i] = _stack[i + 1];
		}
		_stack[N - 1] = value;
	}

	Type sum() {
		Type result = Type{};
		for (int i = 0; i < N; i++) {
			result += _stack[i];
		}
		return result;
	}

	size_t size() {
		return N;
	}

private:
	Type _stack[N];
};

int main(const int argc, const char* const argv[]) {
	try {

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
			throw cppogl::Exception("failed to initialize glew: " + std::string((const char*)glewGetErrorString(err)), EXCEPT_DETAIL_DEFAULT);
		}

		cppogl::Application app = cppogl::Application("cppogl", window);

		app.initialize();

		cppogl::sShaderProgram basic3D = cppogl::sShaderProgram(new cppogl::ShaderProgram("basic3D", "shader/basic3D.vert", "shader/basic3D.frag"));
		app.addShader(basic3D);
		cppogl::sShaderProgram textured3D = cppogl::sShaderProgram(new cppogl::ShaderProgram("textured3D", "shader/textured3D.vert", "shader/textured3D.frag"));
		app.addShader(textured3D);
		cppogl::sShaderProgram text = cppogl::sShaderProgram(new cppogl::ShaderProgram("text", "shader/text.vert", "shader/text.frag"));
		app.addShader(text);
		cppogl::sShaderProgram interface = cppogl::sShaderProgram(new cppogl::ShaderProgram("interface", "shader/interface.vert", "shader/interface.frag"));
		app.addShader(interface);

		cppogl::sFontFamily roboto = std::make_shared<cppogl::FontFamily>(cppogl::FontFamily("roboto", {
			{ cppogl::FontType::REGULAR, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Regular.ttf")) },
			{ cppogl::FontType::BOLD, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Bold.ttf")) },
			{ cppogl::FontType::ITALIC, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Italic.ttf")) },
			{ cppogl::FontType::ITALIC_BOLD, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-BoldItalic.ttf")) },
			{ cppogl::FontType::LIGHT, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-Light.ttf")) },
			{ cppogl::FontType::ITALIC_LIGHT, std::make_shared<cppogl::Font>(cppogl::Font(window, "res/font/Roboto/Roboto-LightItalic.ttf")) } }));

		app.addResource(roboto);

		std::shared_ptr<cppogl::NoClipCamera> camera = std::shared_ptr<cppogl::NoClipCamera>(new cppogl::NoClipCamera(cppogl::PERSPECTIVE_PROJECTION, window));
		camera->translate(-0.1, 0.0, -0.5);

		cppogl::sRenderableLayer mainLayer = cppogl::sRenderableLayer(new cppogl::RenderableLayer("main", camera));
		app.addLayer(mainLayer);

		cppogl::UI::sLayer uiLayer = cppogl::UI::sLayer(new cppogl::UI::Layer(app.getContext(), "ui", UI_TICK));
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

		auto rect = std::shared_ptr<cppogl::ImageRect>(new cppogl::ImageRect(app.getContext(), "textured3D", 1.0, 1.0, "res/sky.png"));
		rect->translate(0.0, 2.0, 0.0);
		triangleA->id = "rect";
		mainLayer->addRenderable(rect);

		auto triangleB = std::shared_ptr<cppogl::Triangle>(new cppogl::Triangle(
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

		cppogl::TextAttributes attrib{ cppogl::FontType::BOLD, cppogl::UnitValue::parse("16pt"), cppogl::UnitVector2D(300.0f, 0.0f, cppogl::Unit::PT), cppogl::UnitVector2D(0.0, 0.0) };
		auto fpsText = std::shared_ptr<cppogl::Text>(new cppogl::Text(app.getContext(), "text", "roboto", "Framerate: ", attrib));
		fpsText->id = "fpsText";
		uiLayer->addElement(fpsText);
		auto wrapTest = std::shared_ptr<cppogl::Text>(new cppogl::Text(app.getContext(),
			"text",
			"roboto",
			"Hello world! This is a test of word wrapping, will it wrap, mabye.",
			cppogl::TextAttributes{ cppogl::FontType::LIGHT, cppogl::UnitValue::parse("16pt"), cppogl::UnitVector2D(300.0f, 0.0f, cppogl::Unit::PT), cppogl::UnitVector2D(0.0, 0.0) }
		));
		wrapTest->id = "wrapTest";
		uiLayer->addElement(wrapTest);
		auto clickText = std::shared_ptr<cppogl::Text>(new cppogl::Text(app.getContext(), "text", "roboto", "Clicked: 0", attrib));
		clickText->id = "clickText";
		uiLayer->addElement(clickText);

		auto basicButton = std::shared_ptr<cppogl::UI::BasicButton>(new cppogl::UI::BasicButton(app.getContext(), "interface", "roboto", "Click Me!"));
		basicButton->id = "basicButton";
		uiLayer->addElement(basicButton);

		auto alignerAttribs = cppogl::UI::LinearAlignerAttributes{};
		alignerAttribs.spacing = cppogl::UnitValue{ 10.0, cppogl::Unit::PT };
		alignerAttribs.topLeft.x = cppogl::UnitValue{ 10.0f, cppogl::Unit::PT };
		auto aligner = cppogl::UI::sLinearAligner(new cppogl::UI::LinearAligner({
			fpsText,
			wrapTest,
			clickText,
			basicButton
		}, alignerAttribs));
		aligner->id = "aligner";
		uiLayer->addLogicNode(aligner);

		Stack<30, int> framerate;
		int numClicked = 0;
		bool updateText = false;

		cppogl::EventCallback<cppogl::MouseStateMessage> clickListener([&numClicked, &updateText](cppogl::MouseStateMessage* message) {
			numClicked++;
			updateText = true;
		});

		basicButton->registerListener("onclick", &clickListener);

		while (!window->shouldClose()) {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			app.render();
			float period = mainLayer->getFrameDelay();
			if (period != 0.0) {
				int value = (int)(1 / mainLayer->getFrameDelay());
				framerate.push(value);
			}
			
			if (uiLayer->tick()) {
				fpsText->update(std::string("Framerate: ") + std::to_string(static_cast<int>(framerate.sum() / framerate.size())));
			}
			if (updateText) {
				clickText->update(std::string("Clicked: ") + std::to_string(numClicked));
				updateText = false;
			}

			checkGLErrors(0);
			
			app.update();
			if (!uiLayer->raycastHit() && window->getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				camera->grab();
			}
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