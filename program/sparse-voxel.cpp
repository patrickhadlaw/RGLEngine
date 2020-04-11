// sparse-voxel.cpp

#include "rgle.h"

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
		auto shaders = { rgle::Shader::compileFile(rgle::installed_filename("shader/sparse-voxel/sparse-voxel.comp"), GL_COMPUTE_SHADER) };
		auto sparseVoxel = std::make_shared<rgle::ShaderProgram>(
			"sparse-voxel",
			shaders
		);
		app.addShader(sparseVoxel);
		auto sparseVoxelRealize = std::make_shared<rgle::ShaderProgram>(
			"sparse-voxel-realize",
			rgle::installed_filename("shader/sparse-voxel/sparse-voxel-realize.vert"),
			rgle::installed_filename("shader/sparse-voxel/sparse-voxel-realize.frag")
		);
		app.addShader(sparseVoxelRealize);

		auto roboto = std::shared_ptr<rgle::FontFamily>(new rgle::FontFamily("roboto", {
			{ rgle::FontType::REGULAR, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Regular.ttf")) },
			{ rgle::FontType::BOLD, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Bold.ttf")) },
			{ rgle::FontType::ITALIC, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Italic.ttf")) },
			{ rgle::FontType::ITALIC_BOLD, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-BoldItalic.ttf")) },
			{ rgle::FontType::LIGHT, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-Light.ttf")) },
			{ rgle::FontType::ITALIC_LIGHT, std::make_shared<rgle::Font>(window, rgle::installed_filename("res/font/Roboto/Roboto-LightItalic.ttf")) }
		}));

		app.addResource(roboto);

		std::shared_ptr<rgle::SparseVoxelOctree> octree;
		std::shared_ptr<rgle::NoClipSparseVoxelCamera> camera;
		std::shared_ptr<rgle::SparseVoxelRenderer> mainLayer;
		std::shared_ptr<rgle::UI::Layer> uiLayer;
		std::shared_ptr<rgle::Text> fpsText;

		app.executeInContext([&app, &window, &octree, &camera, &mainLayer, &uiLayer, &fpsText]() {
			octree = std::make_shared<rgle::SparseVoxelOctree>();

			camera = std::make_shared<rgle::NoClipSparseVoxelCamera>(0.01f, 1000.0f, glm::radians(60.0f), window);

			mainLayer = std::make_shared<rgle::SparseVoxelRenderer>(
				"mainLayer",
				octree,
				"sparse-voxel",
				"sparse-voxel-realize",
				window->width(),
				window->height(),
				camera
			);
			app.addLayer(mainLayer);
			camera->translate(glm::vec3(-0.25f, -0.25f, -1.0f));
			camera->rotate(0.1f, 0.0f, 0.0f);
			octree->root()->size() = 0.25f;
			octree->root()->color() = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
			octree->root()->update();
			octree->root()->insertChildren({
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
				glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
				glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
				glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
				glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
				glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			});

			uiLayer = std::make_shared<rgle::UI::Layer>("ui");
			app.addLayer(uiLayer);

			rgle::TextAttributes attrib{
				rgle::FontType::BOLD,
				rgle::UnitValue::parse("8pt"),
				rgle::UnitVector2D(300.0f, 0.0f, rgle::Unit::PT),
				rgle::UnitVector2D(0.0, 0.0)
			};
			fpsText = std::make_shared<rgle::Text>("text", "roboto", "Framerate: ", attrib);
			fpsText->id = "fpsText";
			uiLayer->addElement(fpsText);
		});

		Stack<30, int> framerate;
		clock_t lastTime = clock();

		while (!window->shouldClose()) {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			app.render();

			float period = (float) (clock() - lastTime) / CLOCKS_PER_SEC;
			if (period != 0.0) {
				lastTime = clock();
				int value = (int)(1.0f / period);
				framerate.push(value);
			}

			if (uiLayer->tick()) {
				fpsText->update(std::string("Framerate: ") + std::to_string(static_cast<int>(framerate.sum() / framerate.size())));
			}

			app.update();

			if (!uiLayer->raycastHit() && window->getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				camera->grab();
			}
		}
	}
	catch (rgle::Exception& e) {
		std::cin.get();
		return -1;
	}
	catch (std::exception& e) {
		rgle::Exception except = rgle::Exception(e.what(), LOGGER_DETAIL_DEFAULT);
		std::cin.get();
		return -1;
	}
	catch (...) {
		rgle::Exception except = rgle::Exception("UNHANDLED EXCEPTION", LOGGER_DETAIL_DEFAULT);
		std::cin.get();
		return -1;
	}
	return 0;
}