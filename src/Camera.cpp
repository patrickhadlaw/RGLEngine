#include "Camera.h"


rgle::Camera::Camera()
{
	this->_position = glm::vec3(0.0, 0.0, 0.0);
	this->_projection = glm::mat4(1.0f);
	this->_view = glm::mat4(1.0f);
}

rgle::Camera::Camera(CameraType type, std::shared_ptr<Window> window)
{
	if (window == nullptr) {
		throw std::runtime_error("Error: null window handle passed");
	}
	this->_window = window;
	this->_type = type;

	this->_window->registerListener("resize", this);

	this->_position = glm::vec3(0.0, 0.0, 0.0);
	this->_direction = glm::vec3(0.0, 0.0, 1.0);
	this->_up = glm::vec3(0.0, 1.0, 0.0);
	this->_right = glm::vec3(1.0, 0.0, 0.0);

	this->generate(type);
}


rgle::Camera::~Camera()
{
}

void rgle::Camera::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		this->generate(this->_type);
	}
}

void rgle::Camera::update(float deltaT)
{
	glUniformMatrix4fv(_projectionLocation, 1, GL_FALSE, &_projection[0][0]);
	_view = glm::lookAt(_position, _position + _direction, _up);
	glUniformMatrix4fv(_viewLocation, 1, GL_FALSE, &_view[0][0]);
}

void rgle::Camera::bind(std::shared_ptr<ShaderProgram> shader)
{
	glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "projection"), 1, GL_FALSE, &_projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "view"), 1, GL_FALSE, &_view[0][0]);
}

void rgle::Camera::generate(CameraType type)
{
	switch (type) {
	default:
	case ORTHOGONAL_PROJECTION:
		this->_projection = glm::ortho(0.0f, (float)_window->width(), (float)_window->height(), 0.0f, 0.01f, 1000.0f);
		break;
	case PERSPECTIVE_PROJECTION:
		this->_projection = glm::perspective(glm::radians(60.0f), ((float)_window->width() / (float)_window->height()), 0.001f, 100.0f);
		break;
	}
}

void rgle::Camera::translate(float x, float y, float z)
{
	_position.x += x;
	_position.y += y;
	_position.z += z;
}

void rgle::Camera::rotate(float x, float y, float z)
{
	x = -x;
	y = -y;
	z = -z;
	glm::vec3 tempDirection = _direction;
	_direction = glm::normalize(glm::angleAxis(x, _up) * glm::angleAxis(y, _right) * glm::angleAxis(z, _direction)) * _direction;
	_up = glm::normalize(glm::angleAxis(x, _up) * glm::angleAxis(y, _right) * glm::angleAxis(z, tempDirection)) * _up;
	_right = glm::cross(_direction, _up);
}

void rgle::Camera::lookAt(glm::vec3 direction)
{
	
}

glm::vec3 rgle::Camera::position()
{
	return _position;
}

glm::vec3 rgle::Camera::direction()
{
	return _direction;
}

glm::vec3 rgle::Camera::up()
{
	return _up;
}

glm::mat4 rgle::Camera::view()
{
	return _view;
}

glm::mat4 rgle::Camera::projection()
{
	return _projection;
}

rgle::NoClipCamera::NoClipCamera() : Camera()
{
	_isGrabbed = false;
}

rgle::NoClipCamera::NoClipCamera(CameraType type, std::shared_ptr<Window> window) : Camera(type, window)
{
	_isGrabbed = false;
	this->_window = window;
	this->_window->registerListener("mousemove", this);
}

rgle::NoClipCamera::~NoClipCamera()
{
}

void rgle::NoClipCamera::onMessage(std::string eventname, EventMessage * message)
{
	Camera::onMessage(eventname, message);
	if (eventname == "mousemove") {
		MouseMoveMessage* mousemove = dynamic_cast<MouseMoveMessage*>(message);
		_mouse.deltaX = mousemove->mouse.x;
		_mouse.deltaY = mousemove->mouse.y;
	}
}

void rgle::NoClipCamera::update(float deltaT)
{
	if (_window->grabbed()) {
		int state = _window->getKey(GLFW_KEY_ESCAPE);
		if (state == GLFW_PRESS) {
			this->unGrab();
		}
		else {
			this->rotate(_mouse.deltaX / _window->height(), _mouse.deltaY / _window->width(), 0.0);
			_mouse.deltaX = 0.0;
			_mouse.deltaY = 0.0;
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

void rgle::NoClipCamera::grab()
{
	this->_isGrabbed = true;
	_window->grabCursor();
}

void rgle::NoClipCamera::unGrab()
{
	this->_isGrabbed = false;
	_window->ungrabCursor();
}

void rgle::NoClipCamera::moveRelative(float forward, float horizontal, float vertical)
{
	glm::vec3 right = glm::cross(_direction, _up);
	_position += _direction * forward + right * horizontal + _up * vertical;
}

rgle::ViewTransformer::ViewTransformer()
{
}

rgle::ViewTransformer::~ViewTransformer()
{
}

void rgle::ViewTransformer::update(float deltaT)
{
}

void rgle::ViewTransformer::bind(std::shared_ptr<ShaderProgram> program)
{
}

rgle::Viewport::Viewport()
{
}

rgle::Viewport::~Viewport()
{
}

void rgle::Viewport::use()
{
}