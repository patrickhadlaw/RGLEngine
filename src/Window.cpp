#include "Window.h"


std::map<GLFWwindow*, cppogl::Window*> cppogl::Window::_handles = std::map<GLFWwindow*, cppogl::Window*>();

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

	_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (_window == nullptr) {
		throw std::runtime_error("Error: failed to create GLFW window");
	}
	glfwMakeContextCurrent(_window);

	_grabbed = false;

	_handles[_window] = this;

	glfwSetCursorPosCallback(_window, [](GLFWwindow * window, double xpos, double ypos) -> void {
		MouseMoveMessage* message = new MouseMoveMessage;
		message->mouse.x = xpos;
		message->mouse.y = ypos;
		Window::handleEvent(window, "mousemove", message);
	});
	glfwSetMouseButtonCallback(_window, [](GLFWwindow * window, int button, int action, int modifiers) -> void {
		MouseClickMessage* message = new MouseClickMessage;
		message->mouse.button = button;
		message->mouse.action = action;
		message->mouse.modifier = modifiers;
		Window::handleEvent(window, "mouseclick", message);
	});
	glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int mode) -> void {
		KeyboardMessage* message = new KeyboardMessage;
		message->keyboard.key = key;
		message->keyboard.scancode = scancode;
		message->keyboard.action = action;
		message->keyboard.mode = mode;
		Window::handleEvent(window, "keyboard", message);
	});
}

cppogl::Window::Window(const Window & other)
{
	throw Exception("invalid usage of Window class, copying not allowed", EXCEPT_DETAIL_DEFAULT);
}

cppogl::Window::Window(Window && rvalue)
{
	this->_window = rvalue._window;
	rvalue._window = nullptr;
	this->_grabbed = rvalue._grabbed;
	this->_initial = rvalue._initial;
	if (_handles.find(_window) == _handles.end()) {
		throw EventException("invalid window handle", EXCEPT_DETAIL_DEFAULT);
	}
	else {
		_handles[_window] = this;
	}
}

cppogl::Window::~Window()
{
	if (_window != nullptr) {
		glfwDestroyWindow(_window);
		auto it = _handles.find(_window);
		_handles.erase(it);
		_window = nullptr;
	}
}

void cppogl::Window::operator=(Window && rvalue)
{
	this->_window = rvalue._window;
	rvalue._window = nullptr;
	this->_grabbed = rvalue._grabbed;
	this->_initial = rvalue._initial;
	if (_handles.find(_window) == _handles.end()) {
		throw EventException("invalid window handle", EXCEPT_DETAIL_DEFAULT);
	}
	else {
		_handles[_window] = this;
	}
}

int cppogl::Window::width()
{
	int width;
	glfwGetWindowSize(_window, &width, nullptr);
	return width;
}

int cppogl::Window::height()
{
	int height;
	glfwGetWindowSize(_window, nullptr, &height);
	return height;
}

float cppogl::Window::physicalWidth()
{
	int widthMM;
	glfwGetMonitorPhysicalSize(glfwGetWindowMonitor(_window), &widthMM, nullptr);
	return (float)widthMM / 1000.0f;
}

float cppogl::Window::physicalHeight()
{
	int heightMM;
	glfwGetMonitorPhysicalSize(glfwGetWindowMonitor(_window), nullptr, &heightMM);
	return (float)heightMM / 1000.0f;
}

float cppogl::Window::parseUnit(float value, Unit unit)
{
	int widthMM, heightMM;
	GLFWmonitor* monitor = glfwGetWindowMonitor(_window);
	if (monitor == nullptr) {
		monitor = glfwGetPrimaryMonitor();
	}
	glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
	const GLFWvidmode * mode = glfwGetVideoMode(monitor);
	float widthInch = (float)widthMM / 1000 * Conversions::IN_PER_M;
	float heightInch = (float)heightMM / 1000 * Conversions::IN_PER_M;
	float pixelsPerInchX = mode->width / widthInch;
	float pixelsPerInchY = mode->height / heightInch;
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	switch (unit)
	{
	case cppogl::Window::Unit::PT:
		return (pixelsPerInchX / 72) * value;
	case cppogl::Window::Unit::VW:
		return (value / 100.0f) * (float) viewport[2];
	case cppogl::Window::Unit::VH:
		return (value / 100.0f) * (float) viewport[3];
	case cppogl::Window::Unit::WW:
		return (value / 100.0f) * (float) this->width();
	case cppogl::Window::Unit::WH:
		return (value / 100.0f) * (float) this->height();
	case cppogl::Window::Unit::IN:
		return pixelsPerInchX * value;
	case cppogl::Window::Unit::CM:
		return pixelsPerInchX * (1.0f / (Conversions::IN_PER_M * 100.0f))  * value;
	default:
		return value;
	}
}

int cppogl::Window::getKey(int key)
{
	return glfwGetKey(_window, key);
}

int cppogl::Window::getMouseButton(int key)
{
	return glfwGetMouseButton(_window, key);
}

void cppogl::Window::grabCursor()
{
	glfwGetCursorPos(_window, &_initial.x, &_initial.y);
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(_window, 0, 0);
	_grabbed = true;
}

void cppogl::Window::ungrabCursor()
{
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPos(_window, _initial.x, _initial.y);
	_grabbed = false;
}

bool cppogl::Window::grabbed()
{
	return _grabbed;
}

bool cppogl::Window::shouldClose()
{
	return glfwWindowShouldClose(_window);
}

void cppogl::Window::update()
{
	glfwSwapBuffers(_window);
	glfwPollEvents();
}

void cppogl::Window::handleEvent(GLFWwindow* handle, std::string eventname, EventMessage * message)
{
	if (_handles.find(handle) == _handles.end()) {
		throw EventException("invalid window handle for event broadcast", EXCEPT_DETAIL_DEFAULT);
	}
	else {
		Window* window = _handles[handle];
		window->broadcastEvent(eventname, message);
		if (window->_grabbed && eventname == "mousemove") {
			glfwSetCursorPos(window->_window, 0.0, 0.0);
		}
	}
}

cppogl::MouseMoveMessage::MouseMoveMessage()
{
}

cppogl::MouseMoveMessage::~MouseMoveMessage()
{
}

cppogl::MouseClickMessage::MouseClickMessage()
{
}

cppogl::MouseClickMessage::~MouseClickMessage()
{
}

cppogl::MouseRaycastMessage::MouseRaycastMessage()
{
}

cppogl::MouseRaycastMessage::~MouseRaycastMessage()
{
}

cppogl::WindowResizeMessage::WindowResizeMessage()
{
}

cppogl::WindowResizeMessage::~WindowResizeMessage()
{
}

cppogl::KeyboardMessage::KeyboardMessage()
{
}

cppogl::KeyboardMessage::~KeyboardMessage()
{
}
