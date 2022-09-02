// interface.cpp

#include "rgle.h"

glm::vec4 randomColor() {
	return glm::vec4(
		(float)(rand() % 255) / 255,
		(float)(rand() % 255) / 255,
		(float)(rand() % 255) / 255,
		1.0
	);
}

const float UI_TICK = 1.0f / 20.0f;

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

		rgle::Application app = rgle::Application("rgle", window);

		app.initialize();

		auto basic3D = std::make_shared<rgle::gfx::ShaderProgram>(
			"basic3D",
			"shader/basic3D.vert",
			"shader/basic3D.frag"
		);
		app.addShader(basic3D);
		auto textured3D = std::make_shared<rgle::gfx::ShaderProgram>(
			"textured3D",
			"shader/textured3D.vert",
			"shader/textured3D.frag"
		);
		app.addShader(textured3D);
		auto text = std::make_shared<rgle::gfx::ShaderProgram>(
			"text",
			"shader/text.vert",
			"shader/text.frag"
		);
		app.addShader(text);
		auto interface = std::make_shared<rgle::gfx::ShaderProgram>(
			"interface",
			"shader/interface.vert",
			"shader/interface.frag"
		);
		app.addShader(interface);

		auto roboto = std::shared_ptr<rgle::res::FontFamily>(new rgle::res::FontFamily("roboto", {
			{ rgle::res::FontType::REGULAR, std::make_shared<rgle::res::Font>(window, "res/font/Roboto/Roboto-Regular.ttf") },
			{ rgle::res::FontType::BOLD, std::make_shared<rgle::res::Font>(window, "res/font/Roboto/Roboto-Bold.ttf") },
			{ rgle::res::FontType::ITALIC, std::make_shared<rgle::res::Font>(window, "res/font/Roboto/Roboto-Italic.ttf") },
			{ rgle::res::FontType::ITALIC_BOLD, std::make_shared<rgle::res::Font>(window, "res/font/Roboto/Roboto-BoldItalic.ttf") },
			{ rgle::res::FontType::LIGHT, std::make_shared<rgle::res::Font>(window, "res/font/Roboto/Roboto-Light.ttf") },
			{ rgle::res::FontType::ITALIC_LIGHT, std::make_shared<rgle::res::Font>(window, "res/font/Roboto/Roboto-LightItalic.ttf") }
		}));

		app.addResource(roboto);

		std::shared_ptr<rgle::gfx::RenderableLayer> mainLayer;
		std::shared_ptr<rgle::ui::Layer> uiLayer;
		std::shared_ptr<rgle::gfx::NoClipCamera> camera;
		std::shared_ptr<rgle::ui::BasicButton> basicButton;
		std::shared_ptr<rgle::ui::Text> fpsText;
		std::shared_ptr<rgle::ui::Text> clickText;

		app.executeInContext([&app, &window, &mainLayer, &uiLayer, &camera, &basicButton, &fpsText, &clickText]() {
			camera = std::make_shared<rgle::gfx::NoClipCamera>(rgle::gfx::PERSPECTIVE_PROJECTION, window);
			camera->translate(-0.1f, 0.0f, -0.5f);
			camera->lookAt(glm::vec3(0.0f, 0.0f, 1.0f));

			mainLayer = std::make_shared<rgle::gfx::RenderableLayer>("main", camera);
			app.addLayer(mainLayer);

			uiLayer = std::make_shared<rgle::ui::Layer>("ui", UI_TICK);
			app.addLayer(uiLayer);

			srand(clock());
			auto triangleA = std::make_shared<rgle::gfx::Triangle>(
				"basic3D",
				1.0,
				1.0,
				glm::radians(60.0f),
				std::vector { randomColor(), randomColor(), randomColor() }
			);
			triangleA->id = "triangleA";
			mainLayer->addRenderable(triangleA);

			auto rect = std::make_shared<rgle::gfx::ImageRect>("textured3D", 1.0f, 1.0f, "res/sky.png");
			rect->translate(0.0f, 2.0f, 0.0f);
			triangleA->id = "rect";
			mainLayer->addRenderable(rect);

			auto triangleB = std::make_shared<rgle::gfx::Triangle>(
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

			rgle::ui::TextAttributes attrib { rgle::res::FontType::BOLD, rgle::UnitValue::parse("16pt"), rgle::UnitVector2D(300.0f, 0.0f, rgle::Unit::PT), rgle::UnitVector2D(0.0, 0.0) };
			fpsText = std::make_shared<rgle::ui::Text>("text", "roboto", "Framerate: ", attrib);
			fpsText->id = "fpsText";
			uiLayer->addElement(fpsText);
			auto wrapTest = std::make_shared<rgle::ui::Text>(
				"text",
				"roboto",
				"Hello world! This is a test of word wrapping, will it wrap, mabye.",
				rgle::ui::TextAttributes{ rgle::res::FontType::LIGHT, rgle::UnitValue::parse("16pt"), rgle::UnitVector2D(300.0f, 0.0f, rgle::Unit::PT), rgle::UnitVector2D(0.0, 0.0) }
			);
			wrapTest->id = "wrapTest";
			uiLayer->addElement(wrapTest);
			clickText = std::make_shared<rgle::ui::Text>("text", "roboto", "Clicked: 0", attrib);
			clickText->id = "clickText";
			uiLayer->addElement(clickText);

			basicButton = std::make_shared<rgle::ui::BasicButton>("interface", "roboto", "Click Me!");
			basicButton->id = "basicButton";
			uiLayer->addElement(basicButton);

			auto alignerAttribs = rgle::ui::LinearAlignerAttributes{};
			alignerAttribs.spacing = rgle::UnitValue{ 10.0, rgle::Unit::PT };
			alignerAttribs.topLeft.x = rgle::UnitValue{ 10.0f, rgle::Unit::PT };
			auto aligner = std::shared_ptr<rgle::ui::LinearAligner>(new rgle::ui::LinearAligner({
				fpsText,
				wrapTest,
				clickText,
				basicButton
				}, alignerAttribs));
			aligner->id = "aligner";
			uiLayer->addLogicNode(aligner);
		});

		Stack<30, int> framerate;
		int numClicked = 0;
		bool updateText = false;

		rgle::EventCallback<rgle::MouseStateMessage> clickListener([&numClicked, &updateText](rgle::MouseStateMessage*) {
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
	catch (rgle::Exception&) {
		return -1;
	}
	catch (std::exception& e) {
		rgle::Exception except = rgle::Exception(e.what(), LOGGER_DETAIL_DEFAULT);
		return -1;
	}
	catch (...) {
		rgle::Exception except = rgle::Exception("UNHANDLED EXCEPTION", LOGGER_DETAIL_DEFAULT);
		return -1;
	}
	return 0;
}