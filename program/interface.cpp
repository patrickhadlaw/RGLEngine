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

		rgle::initialize();

		auto window = std::make_shared<rgle::Window>(width, height, "RGLEngine");

		glewExperimental = true;
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			throw rgle::Exception("failed to initialize glew: " + std::string((const char*)glewGetErrorString(err)), LOGGER_DETAIL_DEFAULT);
		}

		rgle::Application app = rgle::Application("rgle", window);

		app.initialize();

		auto basic3D = std::make_shared<rgle::ShaderProgram>(
			"basic3D",
			rgle::installed_filename("shader/basic3D.vert"),
			rgle::installed_filename("shader/basic3D.frag")
		);
		app.addShader(basic3D);
		auto textured3D = std::make_shared<rgle::ShaderProgram>(
			"textured3D",
			rgle::installed_filename("shader/textured3D.vert"),
			rgle::installed_filename("shader/textured3D.frag")
		);
		app.addShader(textured3D);
		auto text = std::make_shared<rgle::ShaderProgram>(
			"text",
			rgle::installed_filename("shader/text.vert"),
			rgle::installed_filename("shader/text.frag")
		);
		app.addShader(text);
		auto interface = std::make_shared<rgle::ShaderProgram>(
			"interface",
			rgle::installed_filename("shader/interface.vert"),
			rgle::installed_filename("shader/interface.frag")
		);
		app.addShader(interface);

		auto roboto = std::shared_ptr<rgle::FontFamily>(new rgle::FontFamily("roboto", {
			{ rgle::FontType::REGULAR, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Regular.ttf")) },
			{ rgle::FontType::BOLD, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Bold.ttf")) },
			{ rgle::FontType::ITALIC, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Italic.ttf")) },
			{ rgle::FontType::ITALIC_BOLD, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-BoldItalic.ttf")) },
			{ rgle::FontType::LIGHT, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Light.ttf")) },
			{ rgle::FontType::ITALIC_LIGHT, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-LightItalic.ttf")) }
		}));

		app.addResource(roboto);

		auto camera = std::make_shared<rgle::NoClipCamera>(rgle::PERSPECTIVE_PROJECTION, window);
		camera->translate(-0.1, 0.0, -0.5);

		auto mainLayer = std::make_shared<rgle::RenderableLayer>("main", camera);
		app.addLayer(mainLayer);

		auto uiLayer = std::make_shared<rgle::UI::Layer>(app.getContext(), "ui", UI_TICK);
		app.addLayer(uiLayer);

		srand(clock());
		auto triangleA = std::shared_ptr<rgle::Triangle>(new rgle::Triangle(
			app.getContext(),
			"basic3D",
			1.0,
			1.0,
			glm::radians(60.0f),
			{ randomColor(), randomColor(), randomColor() }
		));
		triangleA->id = "triangleA";
		mainLayer->addRenderable(triangleA);

		auto rect = std::make_shared<rgle::ImageRect>(app.getContext(), "textured3D", 1.0, 1.0, rgle::installed_filename("res/sky.png"));
		rect->translate(0.0, 2.0, 0.0);
		triangleA->id = "rect";
		mainLayer->addRenderable(rect);

		auto triangleB = std::make_shared<rgle::Triangle>(
			app.getContext(),
			"basic3D",
			1.0,
			1.0,
			glm::radians(60.0f),
			glm::vec4(0.0, 0.5, 0.75, 1.0)
		);
		triangleB->id = "triangleB";
		mainLayer->addRenderable(triangleB);
		triangleB->rotate(0.0, 0.0, glm::radians(60.0f));
		triangleB->translate(0.0, 0.0, 1.0);

		rgle::TextAttributes attrib{ rgle::FontType::BOLD, rgle::UnitValue::parse("16pt"), rgle::UnitVector2D(300.0f, 0.0f, rgle::Unit::PT), rgle::UnitVector2D(0.0, 0.0) };
		auto fpsText = std::make_shared<rgle::Text>(app.getContext(), "text", "roboto", "Framerate: ", attrib);
		fpsText->id = "fpsText";
		uiLayer->addElement(fpsText);
		auto wrapTest = std::make_shared<rgle::Text>(
			app.getContext(),
			"text",
			"roboto",
			"Hello world! This is a test of word wrapping, will it wrap, mabye.",
			rgle::TextAttributes{ rgle::FontType::LIGHT, rgle::UnitValue::parse("16pt"), rgle::UnitVector2D(300.0f, 0.0f, rgle::Unit::PT), rgle::UnitVector2D(0.0, 0.0) }
		);
		wrapTest->id = "wrapTest";
		uiLayer->addElement(wrapTest);
		auto clickText = std::make_shared<rgle::Text>(app.getContext(), "text", "roboto", "Clicked: 0", attrib);
		clickText->id = "clickText";
		uiLayer->addElement(clickText);

		auto basicButton = std::make_shared<rgle::UI::BasicButton>(app.getContext(), "interface", "roboto", "Click Me!");
		basicButton->id = "basicButton";
		uiLayer->addElement(basicButton);

		auto alignerAttribs = rgle::UI::LinearAlignerAttributes{};
		alignerAttribs.spacing = rgle::UnitValue{ 10.0, rgle::Unit::PT };
		alignerAttribs.topLeft.x = rgle::UnitValue{ 10.0f, rgle::Unit::PT };
		auto aligner = std::shared_ptr<rgle::UI::LinearAligner>(new rgle::UI::LinearAligner({
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
		rgle::Exception except = rgle::Exception(e.what(), LOGGER_DETAIL_DEFAULT);
		cleanup();
		std::cin.get();
		return -1;
	}
	catch (...) {
		rgle::Exception except = rgle::Exception("UNHANDLED EXCEPTION", LOGGER_DETAIL_DEFAULT);
		cleanup();
		std::cin.get();
		return -1;
	}
	cleanup();
	return 0;
}