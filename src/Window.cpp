#include "Window.h"

cppogl::Window::Window()
{
}

cppogl::Window::Window(const int width, const int height, const char* title)
{
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window == nullptr) {
		throw std::runtime_error("Error: failed to create GLFW window");
	}
	glfwMakeContextCurrent(window);
	
}

cppogl::Window::~Window()
{
}

int cppogl::Window::width()
{
	int width;
	glfwGetWindowSize(window, &width, nullptr);
	return width;
}

int cppogl::Window::height()
{
	int height;
	glfwGetWindowSize(window, nullptr, &height);
	return height;
}