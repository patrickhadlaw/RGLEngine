#include "rgle/gfx/Camera.h"


float rgle::gfx::z_index(float z)
{
	return (std::exp2(z / 1000.0f) - 1.0f) / (std::exp2(z / 1000.0f) + 1.0f);
}

rgle::gfx::Camera::Camera()
{
	this->_position = glm::vec3(0.0, 0.0, 0.0);
	this->_projection = glm::mat4(1.0f);
	this->_view = glm::mat4(1.0f);
}

rgle::gfx::Camera::Camera(CameraType type, std::shared_ptr<Window> window)
{
	RGLE_DEBUG_ASSERT(window != nullptr)
	this->_window = window;
	this->_type = type;

	this->_window->registerListener("resize", this);

	this->_position = glm::vec3(0.0, 0.0, 0.0);
	this->_direction = glm::vec3(0.0, 0.0, -1.0f);
	this->_up = glm::vec3(0.0, 1.0f, 0.0);
	this->_right = glm::vec3(1.0f, 0.0, 0.0);

	this->generate(type);
}


rgle::gfx::Camera::~Camera()
{
}

void rgle::gfx::Camera::onMessage(std::string eventname, EventMessage *)
{
	if (eventname == "resize") {
		this->generate(this->_type);
	}
}

void rgle::gfx::Camera::update(float)
{
	this->_view = glm::lookAt(this->_position, this->_position + this->_direction, this->_up);
}

void rgle::gfx::Camera::bind(std::shared_ptr<ShaderProgram> shader)
{
	glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "projection"), 1, GL_FALSE, &_projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "view"), 1, GL_FALSE, &_view[0][0]);
}

void rgle::gfx::Camera::generate(CameraType type)
{
	this->_view = glm::mat4(1.0f);
	switch (type) {
	default:
	case ORTHOGONAL_PROJECTION:
		this->_projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
		break;
	case PERSPECTIVE_PROJECTION:
		this->_projection = glm::perspective(glm::radians(60.0f), ((float)_window->width() / (float)_window->height()), 0.001f, 100.0f);
		break;
	}
}

void rgle::gfx::Camera::translate(float x, float y, float z)
{
	_position.x += x;
	_position.y += y;
	_position.z += z;
}

void rgle::gfx::Camera::rotate(float x, float y, float z)
{
	x = -x;
	y = -y;
	z = -z;
	glm::vec3 tempDirection = _direction;
	_direction = glm::normalize(glm::angleAxis(x, _up) * glm::angleAxis(y, _right) * glm::angleAxis(z, _direction)) * _direction;
	_up = glm::normalize(glm::angleAxis(x, _up) * glm::angleAxis(y, _right) * glm::angleAxis(z, tempDirection)) * _up;
	_right = glm::cross(_direction, _up);
}

void rgle::gfx::Camera::lookAt(glm::vec3 direction)
{
	this->_direction = direction;
	this->_right = glm::cross(this->_direction, this->_up);
	this->_up = glm::cross(this->_right, this->_direction);
}

void rgle::gfx::Camera::relocate(glm::vec3 position)
{
	this->_position = position;
}

glm::vec3 rgle::gfx::Camera::position()
{
	return _position;
}

glm::vec3 rgle::gfx::Camera::direction()
{
	return _direction;
}

glm::vec3 rgle::gfx::Camera::up()
{
	return _up;
}

glm::mat4 rgle::gfx::Camera::view()
{
	return _view;
}

glm::mat4 rgle::gfx::Camera::projection()
{
	return _projection;
}

rgle::gfx::NoClipCamera::NoClipCamera() : Camera()
{
	_isGrabbed = false;
}

rgle::gfx::NoClipCamera::NoClipCamera(CameraType type, std::shared_ptr<Window> window) : Camera(type, window)
{
	_isGrabbed = false;
	this->_window = window;
	this->_window->registerListener("mousemove", this);
}

rgle::gfx::NoClipCamera::~NoClipCamera()
{
}

void rgle::gfx::NoClipCamera::onMessage(std::string eventname, EventMessage * message)
{
	Camera::onMessage(eventname, message);
	if (eventname == "mousemove") {
		MouseMoveMessage* mousemove = dynamic_cast<MouseMoveMessage*>(message);
		_mouse.deltaX = mousemove->mouse.x;
		_mouse.deltaY = mousemove->mouse.y;
	}
}

void rgle::gfx::NoClipCamera::update(float deltaT)
{
	if (_window->grabbed()) {
		int state = _window->getKey(GLFW_KEY_ESCAPE);
		if (state == GLFW_PRESS) {
			this->unGrab();
		}
		else {
			if (this->_mouse.deltaX != 0.0 || this->_mouse.deltaY != 0.0) {
				this->rotate((float)_mouse.deltaX / _window->height(), (float)_mouse.deltaY / _window->width(), 0.0);
				_mouse.deltaX = 0.0;
				_mouse.deltaY = 0.0;
			}
			if (_window->getKey(GLFW_KEY_W) == GLFW_PRESS) {
				moveRelative(deltaT, 0.0, 0.0);
			}
			if (_window->getKey(GLFW_KEY_A) == GLFW_PRESS) {
				moveRelative(0.0, -deltaT, 0.0);
			}
			if (_window->getKey(GLFW_KEY_S) == GLFW_PRESS) {
				moveRelative(-deltaT, 0.0, 0.0);
			}
			if (_window->getKey(GLFW_KEY_D) == GLFW_PRESS) {
				moveRelative(0.0, deltaT, 0.0);
			}
			if (_window->getKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
				moveRelative(0.0, 0.0, deltaT);
			}
			if (_window->getKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
				moveRelative(0.0, 0.0, -deltaT);
			}
			if (_window->getKey(GLFW_KEY_E) == GLFW_PRESS) {
				rotate(0.0, 0.0, -deltaT);
			}
			if (_window->getKey(GLFW_KEY_Q) == GLFW_PRESS) {
				rotate(0.0, 0.0, deltaT);
			}
		}
	}
	_view = glm::lookAt(_position, _position + _direction, _up);
}

void rgle::gfx::NoClipCamera::grab()
{
	this->_isGrabbed = true;
	_window->grabCursor();
}

void rgle::gfx::NoClipCamera::unGrab()
{
	this->_isGrabbed = false;
	_window->ungrabCursor();
}

void rgle::gfx::NoClipCamera::moveRelative(float forward, float horizontal, float vertical)
{
	glm::vec3 right = glm::cross(_direction, _up);
	_position += _direction * forward + right * horizontal + _up * vertical;
}

rgle::gfx::ViewTransformer::ViewTransformer()
{
}

rgle::gfx::ViewTransformer::~ViewTransformer()
{
}

void rgle::gfx::ViewTransformer::update(float)
{
}

void rgle::gfx::ViewTransformer::bind(std::shared_ptr<ShaderProgram> program)
{
}

rgle::gfx::Viewport::Viewport()
{
}

rgle::gfx::Viewport::Viewport(int width, int height, glm::ivec2 position) : _width(width), _height(height), _position(position)
{
}

rgle::gfx::Viewport::~Viewport()
{
}

void rgle::gfx::Viewport::use()
{
}