#include "rgle/Application.h"
#include "rgle/Platform.h"


void rgle::initialize()
{
	rgle::Logger::message(
		"RGLEngine Version: " +
		std::to_string(RGLE_VERSION_MAJOR) +
		'.' +
		std::to_string(RGLE_VERSION_MINOR) +
		'.' +
		std::to_string(RGLE_VERSION_REVISION)
	);
	rgle::Logger::info("initializing RGLEngine", LOGGER_DETAIL_DEFAULT);
	rgle::Platform::initialize();
}

rgle::Application::Application()
{
}

rgle::Application::Application(std::string app, std::shared_ptr<Window> window) : ContextManager(window, app)
{
}

rgle::Application::~Application()
{
}

void rgle::Application::initialize()
{
	rgle::Logger::info("Initializing application", LOGGER_DETAIL_DEFAULT);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(rgle::debugCallback, 0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

std::shared_ptr<rgle::ContextManager> rgle::Application::getContextManager(std::string id)
{
	return std::shared_ptr<rgle::ContextManager>();
}
