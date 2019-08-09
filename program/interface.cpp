// interface.cpp

#include "rgle.h"

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

		rgle::sWindow window = std::make_shared<rgle::Window>(rgle::Window(width, height, "RGLEngine"));

		glewExperimental = true;
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			throw rgle::Exception("failed to initialize glew: " + std::string((const char*)glewGetErrorString(err)), EXCEPT_DETAIL_DEFAULT);
		}

		rgle::Application app = rgle::Application("rgle", window);

		app.initialize();

		rgle::sShaderProgram basic3D = rgle::sShaderProgram(new rgle::ShaderProgram("basic3D", "shader/basic3D.vert", "shader/basic3D.frag"));
		app.addShader(basic3D);
		rgle::sShaderProgram textured3D = rgle::sShaderProgram(new rgle::ShaderProgram("textured3D", "shader/textured3D.vert", "shader/textured3D.frag"));
		app.addShader(textured3D);
		rgle::sShaderProgram text = rgle::sShaderProgram(new rgle::ShaderProgram("text", "shader/text.vert", "shader/text.frag"));
		app.addShader(text);
		rgle::sShaderProgram interface = rgle::sShaderProgram(new rgle::ShaderProgram("interface", "shader/interface.vert", "shader/interface.frag"));
		app.addShader(interface);

		rgle::sFontFamily roboto = std::make_shared<rgle::FontFamily>(rgle::FontFamily("roboto", {
			{ rgle::FontType::REGULAR, std::make_shared<rgle::Font>(rgle::Font(window, "res/font/Roboto/Roboto-Regular.ttf")) },
			{ rgle::FontType::BOLD, std::make_shared<rgle::Font>(rgle::Font(window, "res/font/Roboto/Roboto-Bold.ttf")) },
			{ rgle::FontType::ITALIC, std::make_shared<rgle::Font>(rgle::Font(window, "res/font/Roboto/Roboto-Italic.ttf")) },
			{ rgle::FontType::ITALIC_BOLD, std::make_shared<rgle::Font>(rgle::Font(window, "res/font/Roboto/Roboto-BoldItalic.ttf")) },
			{ rgle::FontType::LIGHT, std::make_shared<rgle::Font>(rgle::Font(window, "res/font/Roboto/Roboto-Light.ttf")) },
			{ rgle::FontType::ITALIC_LIGHT, std::make_shared<rgle::Font>(rgle::Font(window, "res/font/Roboto/Roboto-LightItalic.ttf")) } }));

		app.addResource(roboto);

		std::shared_ptr<rgle::NoClipCamera> camera = std::shared_ptr<rgle::NoClipCamera>(new rgle::NoClipCamera(rgle::PERSPECTIVE_PROJECTION, window));
		camera->translate(-0.1, 0.0, -0.5);

		rgle::sRenderableLayer mainLayer = rgle::sRenderableLayer(new rgle::RenderableLayer("main", camera));
		app.addLayer(mainLayer);

		rgle::UI::sLayer uiLayer = rgle::UI::sLayer(new rgle::UI::Layer(app.getContext(), "ui", UI_TICK));
		app.addLayer(uiLayer);

		srand(clock());
		std::shared_ptr<rgle::Triangle> triangleA = std::shared_ptr<rgle::Triangle>(new rgle::Triangle(
			app.getContext(),
			"basic3D",
			1.0,
			1.0,
			glm::radians(60.0f),
			{ randomColor(), randomColor(), randomColor() }
		));
		triangleA->id = "triangleA";
		mainLayer->addRenderable(triangleA);

		auto rect = std::shared_ptr<rgle::ImageRect>(new rgle::ImageRect(app.getContext(), "textured3D", 1.0, 1.0, "res/sky.png"));
		rect->translate(0.0, 2.0, 0.0);
		triangleA->id = "rect";
		mainLayer->addRenderable(rect);

		auto triangleB = std::shared_ptr<rgle::Triangle>(new rgle::Triangle(
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

		rgle::TextAttributes attrib{ rgle::FontType::BOLD, rgle::UnitValue::parse("16pt"), rgle::UnitVector2D(300.0f, 0.0f, rgle::Unit::PT), rgle::UnitVector2D(0.0, 0.0) };
		auto fpsText = std::shared_ptr<rgle::Text>(new rgle::Text(app.getContext(), "text", "roboto", "Framerate: ", attrib));
		fpsText->id = "fpsText";
		uiLayer->addElement(fpsText);
		auto wrapTest = std::shared_ptr<rgle::Text>(new rgle::Text(app.getContext(),
			"text",
			"roboto",
			"Hello world! This is a test of word wrapping, will it wrap, mabye.",
			rgle::TextAttributes{ rgle::FontType::LIGHT, rgle::UnitValue::parse("16pt"), rgle::UnitVector2D(300.0f, 0.0f, rgle::Unit::PT), rgle::UnitVector2D(0.0, 0.0) }
		));
		wrapTest->id = "wrapTest";
		uiLayer->addElement(wrapTest);
		auto clickText = std::shared_ptr<rgle::Text>(new rgle::Text(app.getContext(), "text", "roboto", "Clicked: 0", attrib));
		clickText->id = "clickText";
		uiLayer->addElement(clickText);

		auto basicButton = std::shared_ptr<rgle::UI::BasicButton>(new rgle::UI::BasicButton(app.getContext(), "interface", "roboto", "Click Me!"));
		basicButton->id = "basicButton";
		uiLayer->addElement(basicButton);

		auto alignerAttribs = rgle::UI::LinearAlignerAttributes{};
		alignerAttribs.spacing = rgle::UnitValue{ 10.0, rgle::Unit::PT };
		alignerAttribs.topLeft.x = rgle::UnitValue{ 10.0f, rgle::Unit::PT };
		auto aligner = rgle::UI::sLinearAligner(new rgle::UI::LinearAligner({
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

		rgle::EventCallback<rgle::MouseStateMessage> clickListener([&numClicked, &updateText](rgle::MouseStateMessage* message) {
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
	catch (rgle::Exception& e) {
		cleanup();
		std::cin.get();
		return -1;
	}
	catch (std::exception& e) {
		rgle::Exception except = rgle::Exception(e.what(), EXCEPT_DETAIL_DEFAULT);
		std::cout << except.message();
		cleanup();
		std::cin.get();
		return -1;
	}
	catch (...) {
		rgle::Exception except = rgle::Exception("UNHANDLED EXCEPTION", EXCEPT_DETAIL_DEFAULT);
		std::cout << except.message();
		cleanup();
		std::cin.get();
		return -1;
	}
	cleanup();
	return 0;
}