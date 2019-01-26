#include "Camera.h"


cppogl::Camera::Camera()
{
	this->position = glm::vec3(0.0, 0.0, 0.0);
	this->projection = glm::mat4(1.0f);
	this->view = glm::mat4(1.0f);
}

cppogl::Camera::Camera(CameraType type, sWindow window)
{
	if (window == nullptr) {
		throw std::runtime_error("Error: null window handle passed");
	}
	this->window = window;
	this->type = type;

	this->window->registerListener("resize", this);

	this->position = glm::vec3(0.0, 0.0, 0.0);
	this->direction = glm::vec3(0.0, 0.0, 1.0);
	this->up = glm::vec3(0.0, 1.0, 0.0);
	this->right = glm::vec3(1.0, 0.0, 0.0);

	this->generate(type);
}


cppogl::Camera::~Camera()
{
}

void cppogl::Camera::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		this->generate(this->type);
	}
}

void cppogl::Camera::update(float deltaT)
{
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
	view = glm::lookAt(position, position + direction, up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
}

void cppogl::Camera::bind(sShaderProgram shader)
{
	glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "view"), 1, GL_FALSE, &view[0][0]);
}

void cppogl::Camera::generate(CameraType type)
{
	switch (type) {
	default:
	case ORTHOGONAL_PROJECTION:
		this->projection = glm::ortho(0.0f, (float)window->width(), (float)window->height(), 0.0f, 0.01f, 1000.0f);
		break;
	case PERSPECTIVE_PROJECTION:
		this->projection = glm::perspective(glm::radians(60.0f), ((float)window->width() / (float)window->height()), 0.001f, 100.0f);
		break;
	}
}

void cppogl::Camera::translate(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
}

void cppogl::Camera::rotate(float x, float y, float z)
{
	x = -x;
	y = -y;
	z = -z;
	glm::vec3 tempDirection = direction;
	direction = glm::normalize(glm::angleAxis(x, up) * glm::angleAxis(y, right) * glm::angleAxis(z, direction)) * direction;
	up = glm::normalize(glm::angleAxis(x, up) * glm::angleAxis(y, right) * glm::angleAxis(z, tempDirection)) * up;
	right = glm::cross(direction, up);
}

void cppogl::Camera::lookAt(glm::vec3 direction)
{
	
}

cppogl::NoClipCamera::NoClipCamera() : Camera()
{
	isGrabbed = false;
}

cppogl::NoClipCamera::NoClipCamera(CameraType type, sWindow window) : Camera(type, window)
{
	isGrabbed = false;
	this->window = window;
	this->window->registerListener("mousemove", this);
}

cppogl::NoClipCamera::~NoClipCamera()
{
}

void cppogl::NoClipCamera::onMessage(std::string eventname, EventMessage * message)
{
	Camera::onMessage(eventname, message);
	if (eventname == "mousemove") {
		MouseMoveMessage* mousemove = dynamic_cast<MouseMoveMessage*>(message);
		_mouse.deltaX = mousemove->mouse.x;
		_mouse.deltaY = mousemove->mouse.y;
	}
}

void cppogl::NoClipCamera::update(float deltaT)
{
	if (window->grabbed()) {
		int state = window->getKey(GLFW_KEY_ESCAPE);
		if (state == GLFW_PRESS) {
			this->unGrab();
		}
		else {
			this->rotate(_mouse.deltaX / window->height(), _mouse.deltaY / window->width(), 0.0);
			_mouse.deltaX = 0.0;
			_mouse.deltaY = 0.0;
			if (window->getKey(GLFW_KEY_W) == GLFW_PRESS) {
				moveRelative(deltaT, 0.0, 0.0);
			}
			if (window->getKey(GLFW_KEY_A) == GLFW_PRESS) {
				moveRelative(0.0, -deltaT, 0.0);
			}
			if (window->getKey(GLFW_KEY_S) == GLFW_PRESS) {
				moveRelative(-deltaT, 0.0, 0.0);
			}
			if (window->getKey(GLFW_KEY_D) == GLFW_PRESS) {
				moveRelative(0.0, deltaT, 0.0);
			}
			if (window->getKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
				moveRelative(0.0, 0.0, deltaT);
			}
			if (window->getKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
				moveRelative(0.0, 0.0, -deltaT);
			}
			if (window->getKey(GLFW_KEY_E) == GLFW_PRESS) {
				rotate(0.0, 0.0, -deltaT);
			}
			if (window->getKey(GLFW_KEY_Q) == GLFW_PRESS) {
				rotate(0.0, 0.0, deltaT);
			}
		}
	}
	view = glm::lookAt(position, position + direction, up);
}

void cppogl::NoClipCamera::grab()
{
	this->isGrabbed = true;
	window->grabCursor();
}

void cppogl::NoClipCamera::unGrab()
{
	this->isGrabbed = false;
	window->ungrabCursor();
}

void cppogl::NoClipCamera::moveRelative(float forward, float horizontal, float vertical)
{
	glm::vec3 right = glm::cross(direction, up);
	position += direction * forward + right * horizontal + up * vertical;
}

cppogl::ViewTransformer::ViewTransformer()
{
}

cppogl::ViewTransformer::~ViewTransformer()
{
}

void cppogl::ViewTransformer::update(float deltaT)
{
}

void cppogl::ViewTransformer::bind(sShaderProgram program)
{
}

cppogl::Viewport::Viewport()
{
}

cppogl::Viewport::~Viewport()
{
}

void cppogl::Viewport::use()
{
}