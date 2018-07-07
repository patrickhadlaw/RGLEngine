#include "Camera.h"


cppogl::Camera::Camera()
{
	this->position = glm::vec3(0.0, 0.0, 0.0);
	this->projection = glm::mat4(1.0f);
	this->view = glm::mat4(1.0f);
}

cppogl::Camera::Camera(CameraType type, sWindow window, sShaderProgram program)
{
	if (window == nullptr) {
		throw std::runtime_error("Error: null window handle passed");
	}
	if (program == nullptr) {
		throw std::runtime_error("Error: null shader handle passed");
	}
	this->window = window;

	this->position = glm::vec3(0.0, 0.0, 0.0);
	this->direction = glm::vec3(0.0, 0.0, 1.0);
	this->up = glm::vec3(0.0, 1.0, 0.0);
	this->right = glm::vec3(1.0, 0.0, 0.0);

	this->generate(type);
	
	viewLocation = glGetUniformLocation(program->id(), "view");
	projectionLocation = glGetUniformLocation(program->id(), "projection");
}


cppogl::Camera::~Camera()
{
}

void cppogl::Camera::update(float deltaT)
{
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
	glm::mat4 view = glm::lookAt(position, position + direction, up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
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

cppogl::NoClipCamera::NoClipCamera(CameraType type, sWindow window, sShaderProgram program) : Camera(type, window, program)
{
	isGrabbed = false;
	this->window = window;
}

cppogl::NoClipCamera::~NoClipCamera()
{
}

void cppogl::NoClipCamera::update(float deltaT)
{
	if (isGrabbed) {
		int state = glfwGetKey(window->window, GLFW_KEY_ESCAPE);
		if (state == GLFW_PRESS) {
			isGrabbed = false;
			glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPos(window->window, 0, 0);
		}
		else {
			double deltaX, deltaY;
			glfwGetCursorPos(window->window, &deltaX, &deltaY);
			this->rotate(deltaX / window->height(), deltaY / window->width(), 0.0);
			glfwSetCursorPos(window->window, 0, 0);
			if (glfwGetKey(window->window, GLFW_KEY_W) == GLFW_PRESS) {
				moveRelative(deltaT, 0.0, 0.0);
			}
			if (glfwGetKey(window->window, GLFW_KEY_A) == GLFW_PRESS) {
				moveRelative(0.0, -deltaT, 0.0);
			}
			if (glfwGetKey(window->window, GLFW_KEY_S) == GLFW_PRESS) {
				moveRelative(-deltaT, 0.0, 0.0);
			}
			if (glfwGetKey(window->window, GLFW_KEY_D) == GLFW_PRESS) {
				moveRelative(0.0, deltaT, 0.0);
			}
			if (glfwGetKey(window->window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				moveRelative(0.0, 0.0, deltaT);
			}
			if (glfwGetKey(window->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
				moveRelative(0.0, 0.0, -deltaT);
			}
			if (glfwGetKey(window->window, GLFW_KEY_E) == GLFW_PRESS) {
				rotate(0.0, 0.0, -deltaT);
			}
			if (glfwGetKey(window->window, GLFW_KEY_Q) == GLFW_PRESS) {
				rotate(0.0, 0.0, deltaT);
			}
		}
	}
	else {
		int state = glfwGetMouseButton(window->window, GLFW_MOUSE_BUTTON_LEFT);
		if (state == GLFW_PRESS) {
			isGrabbed = true;
			glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
	glm::mat4 view = glm::lookAt(position, position + direction, up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
}

void cppogl::NoClipCamera::moveRelative(float forward, float horizontal, float vertical)
{
	glm::vec3 right = glm::cross(direction, up);
	position += direction * forward + right * horizontal + up * vertical;
}