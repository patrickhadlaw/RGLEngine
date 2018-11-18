#include "Window.h"


std::map<GLFWwindow*, cppogl::Window*> cppogl::Window::_handles = std::map<GLFWwindow*, cppogl::Window*>();

cppogl::Unit cppogl::unitFromPostfix(std::string string)
{
	if (string == "px")
		return Unit::PX;
	else if (string == "nd")
		return Unit::ND;
	else if (string == "pt")
		return Unit::PT;
	else if (string == "vw")
		return Unit::VW;
	else if (string == "vh")
		return Unit::VH;
	else if (string == "ww")
		return Unit::WW;
	else if (string == "wh")
		return Unit::WH;
	else if (string == "in")
		return Unit::IN;
	else if (string == "cm")
		return Unit::CM;
	else
		return Unit::ERROR;
}

std::string cppogl::postfixFromUnit(Unit unit)
{
	switch (unit) {
	case Unit::PX:
		return "px";
	case Unit::ND:
		return "nd";
	case Unit::PT:
		return "pt";
	case Unit::VW:
		return "vw";
	case Unit::VH:
		return "vh";
	case Unit::WW:
		return "ww";
	case Unit::WH:
		return "wh";
	case Unit::IN:
		return "in";
	case Unit::CM:
		return "cm";
	default:
		return "";
	}
}

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
	glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height) -> void {
		WindowResizeMessage* message = new WindowResizeMessage;
		message->window.width = width;
		message->window.height = height;
		glViewport(0, 0, width, height);
		Window::handleEvent(window, "resize", message);
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

float cppogl::Window::pixelValue(float value, Unit unit, Direction direction)
{
	int widthMM, heightMM;
	GLFWmonitor* monitor = glfwGetWindowMonitor(_window);
	if (monitor == nullptr) {
		monitor = glfwGetPrimaryMonitor();
	}
	glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
	const GLFWvidmode * mode = glfwGetVideoMode(monitor);
	float inchDim = (direction == X ? (float)widthMM / 1000 : (float)heightMM / 1000) * Conversions::IN_PER_M;
	float pixelsPerInch = (direction == X ? mode->width : mode->height) / inchDim;
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	switch (unit)
	{
	case cppogl::Unit::PT:
		return (pixelsPerInch / 72) * value;
	case cppogl::Unit::ND:
		return value * (direction == X ? viewport[2] : viewport[3]) / 2;
	case cppogl::Unit::VW:
		return (value / 100.0f) * (float) viewport[2];
	case cppogl::Unit::VH:
		return (value / 100.0f) * (float) viewport[3];
	case cppogl::Unit::WW:
		return (value / 100.0f) * (float) this->width();
	case cppogl::Unit::WH:
		return (value / 100.0f) * (float) this->height();
	case cppogl::Unit::IN:
		return pixelsPerInch * value;
	case cppogl::Unit::CM:
		return pixelsPerInch * (1.0f / (Conversions::IN_PER_M * 100.0f))  * value;
	case cppogl::Unit::ERROR:
		throw Exception("invalid unit conversion", EXCEPT_DETAIL_DEFAULT);
	default:
		return value;
	}
}

float cppogl::Window::pixelValue(UnitValue value, Direction direction)
{
	return pixelValue(value.value, value.unit, direction);
}

float cppogl::Window::resolve(UnitValue value, Direction direction)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float divisor = direction == X ? viewport[2] : viewport[3];
	switch (value.unit) {
	case Unit::PT:
	case Unit::CM:
	case Unit::IN:
	case Unit::PX:
		return (this->pixelValue(value.value, value.unit, direction) / divisor) * 2;
		break;
	default:
		return value.value;
		break;
	}
}

float cppogl::Window::convert(float value, Unit from, Unit to)
{
	return convert(UnitValue{ value, from }, to).value;
}

cppogl::UnitValue cppogl::Window::convert(UnitValue value, Unit to)
{
	float pixel = pixelValue(value);
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
	switch (to)
	{
	case cppogl::Unit::PT:
		value.value = (72 / pixelsPerInchX) * pixel;
		break;
	case cppogl::Unit::ND:
		value.value = pixel * 2 / this->width();
		break;
	case cppogl::Unit::VW:
		value.value = (pixel / (float)viewport[2]) * 100.0f;
		break;
	case cppogl::Unit::VH:
		value.value = (pixel / (float)viewport[3]) * 100.0f;
		break;
	case cppogl::Unit::WW:
		value.value = (pixel / (float)this->width()) * 100.0f;
		break;
	case cppogl::Unit::WH:
		value.value = (pixel / (float)this->height()) * 100.0f;
		break;
	case cppogl::Unit::IN:
		value.value = pixel / pixelsPerInchX;
		break;
	case cppogl::Unit::CM:
		value.value = pixel / pixelsPerInchX * (1.0f / (Conversions::IN_PER_M * 100.0f));
		break;
	case cppogl::Unit::ERROR:
		throw Exception("invalid unit conversion", EXCEPT_DETAIL_DEFAULT);
		break;
	default:
		return value;
	}
	value.unit = to;
	return value;
}

float cppogl::Window::pointValue(float convert, Direction direction)
{
	int widthMM, heightMM;
	GLFWmonitor* monitor = glfwGetWindowMonitor(_window);
	if (monitor == nullptr) {
		monitor = glfwGetPrimaryMonitor();
	}
	glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
	const GLFWvidmode * mode = glfwGetVideoMode(monitor);
	float dim = direction == X ? (float)widthMM / 1000 * Conversions::IN_PER_M : (float)heightMM / 1000 * Conversions::IN_PER_M;
	float pixelDim = direction == X ? (float)mode->width : (float)mode->height;
	return (convert / (pixelDim / dim)) * 72;
}

float cppogl::Window::normalizeX(float value)
{
	return value / this->width();
}

float cppogl::Window::normalizeY(float value)
{
	return value / this->height();
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

cppogl::UnitValue& cppogl::UnitValue::parse(std::string parse)
{
	enum ParseState {
		VALUE_SIGN,
		INTEGRAL,
		DECIMAL,
		EXPONENT_SIGN,
		EXPONENT,
		POSTFIX,
		TRAILING
	};
	ParseState currentState = INTEGRAL;
	bool negateValue = false;
	bool negateExponent = false;
	int value = 0;
	float decimal = 0.0f;
	int exponent = 0;
	std::string postfix = "";
	for (int i = 0; i < parse.length(); i++) {
		char& c = parse[i];
		switch (currentState) {
		case VALUE_SIGN:
			if (c == '+') {
			}
			else if (c == '-') {
				negateValue = true;
			}
			currentState = INTEGRAL;
		case INTEGRAL:
			if (c >= '0' && c <= '9') {
				value *= 10;
				value += (float)(c - '0');
			}
			else if (c == '.') {
				currentState = DECIMAL;
			}
			else if (tolower(c) >= 'a' && tolower(c) <= 'z' && i > 0) {
				postfix.push_back(c);
				currentState = POSTFIX;
			}
			else if (c == ' ') {

			}
			else {
				return UnitValue{ 0.0f, Unit::ERROR };
			}
			break;
		case DECIMAL:
			if (c >= '0' && c <= '9') {
				decimal += (float)(c - '0');
				decimal /= 10;
			}
			else if (c == 'e') {
				currentState = EXPONENT;
			}
			else if (tolower(c) >= 'a' && tolower(c) <= 'z') {
				postfix.push_back(c);
				currentState = POSTFIX;
			}
			else if (c == ' ') {

			}
			else {
				return UnitValue{ 0.0f, Unit::ERROR };
			}
			break;
		case EXPONENT_SIGN:
			if (c == '+') {

			}
			else if (c == '-') {
				negateExponent = true;
			}
			currentState = EXPONENT;
		case EXPONENT:
			if (c >= '0' && c <= '9') {
				exponent *= 10;
				exponent += (float)(c - '0');
			}
			else if (tolower(c) >= 'a' && tolower(c) <= 'z') {
				postfix.push_back(c);
				currentState = POSTFIX;
			}
			else if (c == ' ') {

			}
			else {
				return UnitValue{ 0.0f, Unit::ERROR };
			}
			break;
		case POSTFIX:
			if (tolower(c) >= 'a' && tolower(c) <= 'z') {
				postfix.push_back(c);
			}
			else if (c == ' ') {
				currentState = TRAILING;
			}
			else {
				return UnitValue{ 0.0f, Unit::ERROR };
			}
			break;
		case TRAILING:
			if (c == ' ') {

			}
			else {
				return UnitValue{ 0.0f, Unit::ERROR };
			}
		}
	}
	UnitValue unitvalue;
	unitvalue.unit = unitFromPostfix(postfix);
	unitvalue.value = (float)value + decimal;
	if (negateValue) {
		unitvalue.value = -unitvalue.value;
	}
	if (negateExponent) {
		exponent = -exponent;
	}
	unitvalue.value = unitvalue.value * powf(10, exponent);
	return unitvalue;
}

float cppogl::UnitValue::resolvePixelValue(sWindow window, Window::Direction direction)
{
	return 0.0f;
}

float cppogl::UnitValue::resolve(sWindow window, Window::Direction direction)
{
	return window->resolve(*this, direction);
}

cppogl::UnitExpression::UnitExpression()
{
	this->left = UnitValue{ 0.0f, Unit::ND };
	this->operation = Operation::VALUE;
	this->right = nullptr;
}

cppogl::UnitExpression::UnitExpression(UnitValue value)
{
	this->left = value;
	this->operation = Operation::VALUE;
	this->right = nullptr;
}

cppogl::UnitExpression::UnitExpression(UnitValue left, char op, UnitValue right)
{
	this->left = left;
	switch (op) {
	case '+':
		this->operation = Operation::ADDITION;
		break;
	case '-':
		this->operation = Operation::SUBTRACTION;
		break;
	case '*':
		this->operation = Operation::MULTIPLICATION;
		break;
	case '/':
		this->operation = Operation::DIVISION;
		break;
	default:
		throw Exception("invalid unit expression", EXCEPT_DETAIL_DEFAULT);
	}
	this->right = new UnitExpression(right);
}

cppogl::UnitExpression::UnitExpression(UnitValue left, char op, UnitExpression right)
{
	this->left = left;
	switch (op) {
	case '+':
		this->operation = Operation::ADDITION;
		break;
	case '-':
		this->operation = Operation::SUBTRACTION;
		break;
	case '*':
		this->operation = Operation::MULTIPLICATION;
		break;
	case '/':
		this->operation = Operation::DIVISION;
		break;
	default:
		throw Exception("invalid unit expression", EXCEPT_DETAIL_DEFAULT);
	}
	this->right = new UnitExpression(right);
}

cppogl::UnitExpression::UnitExpression(const UnitExpression & other)
{
	this->left = other.left;
	this->operation = other.operation;
	if (other.right != nullptr) {
		this->right = new UnitExpression(*other.right);
	}
	else {
		this->right = nullptr;
	}
}

cppogl::UnitExpression::UnitExpression(UnitExpression && rvalue)
{
	this->left = rvalue.left;
	this->operation = rvalue.operation;
	this->right = rvalue.right;
	rvalue.right = nullptr;
}

cppogl::UnitExpression::~UnitExpression()
{
	delete this->right;
	this->right = nullptr;
}

void cppogl::UnitExpression::operator=(const UnitExpression & other)
{
	this->left = other.left;
	this->operation = other.operation;
	if (other.right != nullptr) {
		this->right = new UnitExpression(*other.right);
	}
	else {
		this->right = nullptr;
	}
}

void cppogl::UnitExpression::operator=(UnitExpression && rvalue)
{
	delete this->right;
	this->left = rvalue.left;
	this->operation = rvalue.operation;
	this->right = rvalue.right;
	rvalue.right = nullptr;
}

void cppogl::UnitExpression::operator=(UnitValue & value)
{
	this->left = value;
	this->operation = Operation::VALUE;
	this->right = nullptr;
}

cppogl::UnitExpression cppogl::UnitExpression::operator+(UnitValue & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::ADDITION);
	return exp;
}

cppogl::UnitExpression cppogl::UnitExpression::operator+(UnitExpression & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::ADDITION);
	return exp;
}

cppogl::UnitExpression cppogl::UnitExpression::operator-(UnitValue & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::SUBTRACTION);
	return exp;
}

cppogl::UnitExpression cppogl::UnitExpression::operator-(UnitExpression & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::SUBTRACTION);
	return exp;
}

cppogl::UnitExpression cppogl::UnitExpression::operator/(UnitValue & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::DIVISION);
	return exp;
}

cppogl::UnitExpression cppogl::UnitExpression::operator/(UnitExpression & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::DIVISION);
	return exp;
}

cppogl::UnitExpression cppogl::UnitExpression::operator*(UnitValue & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::MULTIPLICATION);
	return exp;
}

cppogl::UnitExpression cppogl::UnitExpression::operator*(UnitExpression & value)
{
	UnitExpression exp = UnitExpression(*this);
	exp._extend(value, Operation::MULTIPLICATION);
	return exp;
}

void cppogl::UnitExpression::operator+=(UnitValue & value)
{
	this->_extend(value, Operation::ADDITION);
}

void cppogl::UnitExpression::operator+=(UnitExpression & value)
{
	this->_extend(value, Operation::ADDITION);
}

void cppogl::UnitExpression::operator-=(UnitValue & value)
{
	this->_extend(value, Operation::SUBTRACTION);
}

void cppogl::UnitExpression::operator-=(UnitExpression & value)
{
	this->_extend(value, Operation::SUBTRACTION);
}

void cppogl::UnitExpression::operator/=(UnitValue & value)
{
	this->_extend(value, Operation::DIVISION);
}

void cppogl::UnitExpression::operator/=(UnitExpression & value)
{
	this->_extend(value, Operation::DIVISION);
}

void cppogl::UnitExpression::operator*=(UnitValue & value)
{
	this->_extend(value, Operation::MULTIPLICATION);
}

void cppogl::UnitExpression::operator*=(UnitExpression & value)
{
	this->_extend(value, Operation::MULTIPLICATION);
}

bool cppogl::UnitExpression::lessThan(UnitExpression & other, sWindow window)
{
	return this->resolve(window) <= other.resolve(window);
}

bool cppogl::UnitExpression::greaterThan(UnitExpression & other, sWindow window)
{
	return this->resolve(window) >= other.resolve(window);
}

bool cppogl::UnitExpression::isZero()
{
	if (this->operation == Operation::VALUE) {
		if (this->left.value == 0.0f) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return this->right->isZero();
	}
}

float cppogl::UnitExpression::resolvePixelValue(sWindow window, Window::Direction direction)
{
	return 0.0f;
}

float cppogl::UnitExpression::resolve(sWindow window, Window::Direction direction)
{
	switch (this->operation) {
	case Operation::ADDITION:
		return this->left.resolve(window, direction) + this->right->resolve(window, direction);
		break;
	case Operation::SUBTRACTION:
		return this->left.resolve(window, direction) - this->right->resolve(window, direction);
		break;
	case Operation::MULTIPLICATION:
		return this->left.resolve(window, direction) * this->right->resolve(window, direction);
		break;
	case Operation::DIVISION:
		return this->left.resolve(window, direction) / this->right->resolve(window, direction);
		break;
	case Operation::VALUE:
		return this->left.resolve(window, direction);
		break;
	default:
		throw Exception("invalid operation type", EXCEPT_DETAIL_DEFAULT);
	}
}

void cppogl::UnitExpression::_extend(UnitValue & value, Operation op)
{
	this->_extend(UnitExpression(value), op);
}

void cppogl::UnitExpression::_extend(UnitExpression & value, Operation op)
{
	if (this->right == nullptr) {
		this->operation = op;
		this->right = new UnitExpression(value);
	}
	else {
		UnitExpression* ptr = this->right;
		while (ptr->right != nullptr) {
			ptr = ptr->right;
		}
		ptr->operation = op;
		ptr->right = new UnitExpression(value);
	}
}

cppogl::UnitVector2D::UnitVector2D()
{
}

cppogl::UnitVector2D::UnitVector2D(float x, float y, Unit unit)
{
	this->x = UnitValue{ x, unit };
	this->y = UnitValue{ y, unit };
}

cppogl::UnitVector2D & cppogl::UnitVector2D::parse(std::string parse)
{
	return UnitVector2D();
}

void cppogl::UnitVector2D::operator+=(UnitVector2D & other)
{
	this->x += other.x;
	this->x += other.y;
}

glm::vec2 cppogl::UnitVector2D::resolve(sWindow window)
{
	return glm::vec2(this->x.resolve(window, Window::Direction::X), this->y.resolve(window, Window::Direction::Y));
}

cppogl::UnitVector3D::UnitVector3D()
{
}

cppogl::UnitVector3D::UnitVector3D(float x, float y, float z, Unit unit)
{
	this->x = UnitValue{ x, unit };
	this->y = UnitValue{ y, unit };
	this->z = UnitValue{ z, unit };
}

cppogl::UnitVector3D & cppogl::UnitVector3D::parse(std::string parse)
{
	return UnitVector3D();
}

glm::vec3 cppogl::UnitVector3D::resolve(sWindow window)
{
	return glm::vec3(this->x.resolve(window, Window::Direction::X), this->y.resolve(window, Window::Direction::Y), this->z.resolve(window, Window::Direction::Y));
}
