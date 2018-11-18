#include "Application.h"
#include "Platform.h"


void CPPOGL_AbortHandler(int signal_number)
{
	std::cout << "SIGABORT received: " + std::to_string(signal_number) << std::endl;
}

cppogl::Application::Application()
{
}

cppogl::Application::Application(std::string app, sWindow window) : ContextManager(window, app)
{
}

cppogl::Application::~Application()
{
}

void cppogl::Application::initialize()
{
	CPPOGL_Initialize_Platform();
	signal(SIGABRT, CPPOGL_AbortHandler);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(cppogl::debugCallback, 0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

cppogl::sContextManager cppogl::Application::getContextManager(std::string id)
{
	return sContextManager();
}
