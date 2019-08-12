#include "Application.h"
#include "Platform.h"


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

rgle::Application::Application(std::string app, sWindow window) : ContextManager(window, app)
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

rgle::sContextManager rgle::Application::getContextManager(std::string id)
{
	return sContextManager();
}
