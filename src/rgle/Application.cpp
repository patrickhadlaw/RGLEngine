#include "rgle/Application.h"
#include "rgle/Platform.h"


void rgle::initialize()
{
	rgle::Platform::initialize();
	rgle::Logger::message(
		"RGLEngine Version: " +
		std::to_string(RGLE_VERSION_MAJOR) +
		'.' +
		std::to_string(RGLE_VERSION_MINOR) +
		'.' +
		std::to_string(RGLE_VERSION_REVISION)
	);
	rgle::Logger::info("initializing RGLEngine", LOGGER_DETAIL_DEFAULT);
}

rgle::Application::Application()
{
}

rgle::Application::Application(std::string app, std::shared_ptr<Window> window) : gfx::ContextManager(window, app)
{
}

rgle::Application::~Application()
{
}

void rgle::Application::initialize()
{
	rgle::Logger::info("Initializing application: " + this->id, LOGGER_DETAIL_IDENTIFIER(this->id));
	const char* vendor = (const char*)glGetString(GL_VENDOR);
	rgle::Logger::info(std::string("OpenGL Vendor[") + vendor + ']', LOGGER_DETAIL_IDENTIFIER(this->id));
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	rgle::Logger::info(std::string("OpenGL Renderer[") + renderer + ']', LOGGER_DETAIL_IDENTIFIER(this->id));
	const char* version = (const char*)glGetString(GL_VERSION);
	rgle::Logger::info(std::string("OpenGL Version[") + version + ']', LOGGER_DETAIL_IDENTIFIER(this->id));
	const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	rgle::Logger::info(std::string("GLSL Version[") + glslVersion + ']', LOGGER_DETAIL_IDENTIFIER(this->id));
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(rgle::gfx::debugCallback, 0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

std::shared_ptr<rgle::gfx::ContextManager> rgle::Application::getContextManager(std::string managerid)
{
	return std::shared_ptr<rgle::gfx::ContextManager>();
}
