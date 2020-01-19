#include "rgle/Window.h"


std::map<GLFWwindow*, rgle::Window*> rgle::Window::_handles = std::map<GLFWwindow*, rgle::Window*>();

rgle::Unit rgle::unitFromPostfix(std::string string)
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

std::string rgle::postfixFromUnit(Unit unit)
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

rgle::Window::Window() : _cursor(nullptr)
{
}

rgle::Window::Window(const int width, const int height, const char* title) : _cursor(nullptr)
{
	rgle::Logger::info("creating window with title: " + std::string(title), LOGGER_DETAIL_DEFAULT);
	if (glfwInit() == GLFW_FALSE) {
		throw rgle::Exception("failed to initialize glfw", LOGGER_DETAIL_DEFAULT);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (_window == nullptr) {
		throw Exception("failed to create GLFW window", LOGGER_DETAIL_DEFAULT);
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

rgle::Window::Window(const Window & other)
{
	throw Exception("invalid usage of Window class, copying not allowed", LOGGER_DETAIL_DEFAULT);
}

rgle::Window::Window(Window && rvalue)
{
	this->_window = rvalue._window;
	this->_cursor = rvalue._cursor;
	rvalue._window = nullptr;
	this->_grabbed = rvalue._grabbed;
	this->_initial = rvalue._initial;
	if (_handles.find(_window) == _handles.end()) {
		throw EventException("invalid window handle", LOGGER_DETAIL_DEFAULT);
	}
	else {
		_handles[_window] = this;
	}
}

rgle::Window::~Window()
{
	if (_window != nullptr) {
		if (this->_cursor != nullptr) {
			glfwDestroyCursor(_cursor);
		}
		glfwDestroyWindow(_window);
		auto it = _handles.find(_window);
		_handles.erase(it);
		_window = nullptr;
	}
}

void rgle::Window::operator=(Window && rvalue)
{
	if (this->_window != nullptr) {
		glfwDestroyWindow(this->_window);
	}
	this->_window = rvalue._window;
	if (this->_cursor != nullptr) {
		glfwDestroyCursor(this->_cursor);
	}
	this->_cursor = rvalue._cursor;
	rvalue._window = nullptr;
	this->_grabbed = rvalue._grabbed;
	this->_initial = rvalue._initial;
	if (_handles.find(_window) == _handles.end()) {
		throw EventException("invalid window handle", LOGGER_DETAIL_DEFAULT);
	}
	else {
		_handles[_window] = this;
	}
}

int rgle::Window::width()
{
	int width;
	glfwGetWindowSize(_window, &width, nullptr);
	return width;
}

int rgle::Window::height()
{
	int height;
	glfwGetWindowSize(_window, nullptr, &height);
	return height;
}

float rgle::Window::physicalWidth()
{
	int widthMM;
	glfwGetMonitorPhysicalSize(glfwGetWindowMonitor(_window), &widthMM, nullptr);
	return (float)widthMM / 1000.0f;
}

float rgle::Window::physicalHeight()
{
	int heightMM;
	glfwGetMonitorPhysicalSize(glfwGetWindowMonitor(_window), nullptr, &heightMM);
	return (float)heightMM / 1000.0f;
}

float rgle::Window::pixelValue(float value, Unit unit, Direction direction)
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
	case rgle::Unit::PT:
		return (pixelsPerInch / 72) * value;
	case rgle::Unit::ND:
		return value * (direction == X ? viewport[2] : viewport[3]) / 2;
	case rgle::Unit::VW:
		return (value / 100.0f) * (float) viewport[2];
	case rgle::Unit::VH:
		return (value / 100.0f) * (float) viewport[3];
	case rgle::Unit::WW:
		return (value / 100.0f) * (float) this->width();
	case rgle::Unit::WH:
		return (value / 100.0f) * (float) this->height();
	case rgle::Unit::IN:
		return pixelsPerInch * value;
	case rgle::Unit::CM:
		return pixelsPerInch * (1.0f / (Conversions::IN_PER_M * 100.0f))  * value;
	case rgle::Unit::ERROR:
		throw Exception("invalid unit conversion", LOGGER_DETAIL_DEFAULT);
	default:
		return value;
	}
}

float rgle::Window::pixelValue(UnitValue value, Direction direction)
{
	return pixelValue(value.value, value.unit, direction);
}

float rgle::Window::resolve(UnitValue value, Direction direction)
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

float rgle::Window::convert(float value, Unit from, Unit to)
{
	return convert(UnitValue{ value, from }, to).value;
}

rgle::UnitValue rgle::Window::convert(UnitValue value, Unit to)
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
	case rgle::Unit::PT:
		value.value = (72 / pixelsPerInchX) * pixel;
		break;
	case rgle::Unit::ND:
		value.value = pixel * 2 / this->width();
		break;
	case rgle::Unit::VW:
		value.value = (pixel / (float)viewport[2]) * 100.0f;
		break;
	case rgle::Unit::VH:
		value.value = (pixel / (float)viewport[3]) * 100.0f;
		break;
	case rgle::Unit::WW:
		value.value = (pixel / (float)this->width()) * 100.0f;
		break;
	case rgle::Unit::WH:
		value.value = (pixel / (float)this->height()) * 100.0f;
		break;
	case rgle::Unit::IN:
		value.value = pixel / pixelsPerInchX;
		break;
	case rgle::Unit::CM:
		value.value = pixel / pixelsPerInchX * (1.0f / (Conversions::IN_PER_M * 100.0f));
		break;
	case rgle::Unit::ERROR:
		throw Exception("invalid unit conversion", LOGGER_DETAIL_DEFAULT);
		break;
	default:
		return value;
	}
	value.unit = to;
	return value;
}

float rgle::Window::pointValue(float convert, Direction direction)
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

float rgle::Window::normalizeX(float value)
{
	return value / this->width();
}

float rgle::Window::normalizeY(float value)
{
	return value / this->height();
}

int rgle::Window::getKey(int key)
{
	return glfwGetKey(_window, key);
}

int rgle::Window::getMouseButton(int key)
{
	return glfwGetMouseButton(_window, key);
}

glm::vec2 rgle::Window::getCursorPosition()
{
	double x, y = 0.0f;
	glfwGetCursorPos(this->_window, &x, &y);
	return glm::vec2(x, y);
}

void rgle::Window::setInputMode(int mode, int value)
{
	glfwSetInputMode(this->_window, mode, value);
}

void rgle::Window::setCursor(int type)
{
	GLFWcursor* tmp = this->_cursor;
	this->_cursor = glfwCreateStandardCursor(type);
	glfwSetCursor(this->_window, this->_cursor);
	if (tmp != nullptr) {
		glfwDestroyCursor(tmp);
	}
}

void rgle::Window::grabCursor()
{
	glfwGetCursorPos(_window, &_initial.x, &_initial.y);
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(_window, 0, 0);
	_grabbed = true;
}

void rgle::Window::ungrabCursor()
{
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPos(_window, _initial.x, _initial.y);
	_grabbed = false;
}

bool rgle::Window::grabbed()
{
	return _grabbed;
}

bool rgle::Window::shouldClose()
{
	return glfwWindowShouldClose(_window);
}

void rgle::Window::update()
{
	glfwSwapBuffers(_window);
	glfwPollEvents();
}

void rgle::Window::handleEvent(GLFWwindow* handle, std::string eventname, EventMessage * message)
{
	if (_handles.find(handle) == _handles.end()) {
		throw EventException("invalid window handle for event broadcast", LOGGER_DETAIL_DEFAULT);
	}
	else {
		Window* window = _handles[handle];
		window->broadcastEvent(eventname, message);
		if (eventname == "mousemove" && window->_grabbed) {
			glfwSetCursorPos(window->_window, 0.0, 0.0);
		}
	}
}

rgle::MouseMoveMessage::MouseMoveMessage()
{
}

rgle::MouseMoveMessage::~MouseMoveMessage()
{
}

rgle::MouseClickMessage::MouseClickMessage()
{
}

rgle::MouseClickMessage::~MouseClickMessage()
{
}

rgle::WindowResizeMessage::WindowResizeMessage()
{
}

rgle::WindowResizeMessage::~WindowResizeMessage()
{
}

rgle::KeyboardMessage::KeyboardMessage()
{
}

rgle::KeyboardMessage::~KeyboardMessage()
{
}

rgle::UnitValue& rgle::UnitValue::parse(std::string parse)
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

float rgle::UnitValue::resolvePixelValue(std::shared_ptr<Window> window, Window::Direction direction)
{
	return 0.0f;
}

float rgle::UnitValue::resolve(std::shared_ptr<Window> window, Window::Direction direction)
{
	return window->resolve(*this, direction);
}

rgle::UnitExpression::UnitExpression() : _value(UnitValue{}), _operation(Operation::VALUE), _left(nullptr), _right(nullptr)
{
}

rgle::UnitExpression::UnitExpression(UnitValue value)
{
	this->_value = value;
	this->_left = nullptr;
	this->_operation = Operation::VALUE;
	this->_right = nullptr;
}

rgle::UnitExpression::UnitExpression(UnitValue left, char op, UnitValue right)
{
	this->_left = new UnitExpression(left);
	switch (op) {
	case '+':
		this->_operation = Operation::ADDITION;
		break;
	case '-':
		this->_operation = Operation::SUBTRACTION;
		break;
	case '*':
		this->_operation = Operation::MULTIPLICATION;
		break;
	case '/':
		this->_operation = Operation::DIVISION;
		break;
	default:
		throw Exception("invalid unit expression", LOGGER_DETAIL_DEFAULT);
	}
	this->_right = new UnitExpression(right);
}

rgle::UnitExpression::UnitExpression(UnitValue left, char op, UnitExpression right) : UnitExpression(UnitExpression(left), op, right)
{
}

rgle::UnitExpression::UnitExpression(UnitExpression left, char op, UnitExpression right)
{
	this->_left = new UnitExpression(left);
	switch (op) {
	case '+':
		this->_operation = Operation::ADDITION;
		break;
	case '-':
		this->_operation = Operation::SUBTRACTION;
		break;
	case '*':
		this->_operation = Operation::MULTIPLICATION;
		break;
	case '/':
		this->_operation = Operation::DIVISION;
		break;
	default:
		throw Exception("invalid unit expression", LOGGER_DETAIL_DEFAULT);
	}
	this->_right = new UnitExpression(right);
}

rgle::UnitExpression::UnitExpression(const UnitExpression & other)
{
	this->_value = other._value;
	this->_operation = other._operation;
	if (other._left != nullptr) {
		this->_left = new UnitExpression(*other._left);
	}
	else {
		this->_left = nullptr;
	}
	if (other._right != nullptr) {
		this->_right = new UnitExpression(*other._right);
	}
	else {
		this->_right = nullptr;
	}
}

rgle::UnitExpression::UnitExpression(UnitExpression && rvalue)
{
	this->_value = rvalue._value;
	delete this->_left;
	delete this->_right;
	this->_left = rvalue._left;
	rvalue._left = nullptr;
	this->_operation = rvalue._operation;
	this->_right = rvalue._right;
	rvalue._right = nullptr;
}

rgle::UnitExpression::~UnitExpression()
{
	delete this->_left;
	this->_left = nullptr;
	delete this->_right;
	this->_right = nullptr;
}

void rgle::UnitExpression::operator=(const UnitExpression & other)
{
	this->_value = other._value;
	this->_operation = other._operation;
	if (other._left != nullptr) {
		this->_left = new UnitExpression(*other._left);
	}
	else {
		this->_left = nullptr;
	}
	if (other._right != nullptr) {
		this->_right = new UnitExpression(*other._right);
	}
	else {
		this->_right = nullptr;
	}
}

void rgle::UnitExpression::operator=(UnitExpression && rvalue)
{
	this->_value = rvalue._value;
	delete this->_left;
	delete this->_right;
	this->_value = rvalue._value;
	this->_left = rvalue._left;
	rvalue._left = nullptr;
	this->_operation = rvalue._operation;
	this->_right = rvalue._right;
	rvalue._right = nullptr;
}

void rgle::UnitExpression::operator=(UnitValue & value)
{
	this->_value = value;
	this->_left = nullptr;
	this->_operation = Operation::VALUE;
	this->_right = nullptr;
}

rgle::UnitExpression rgle::UnitExpression::operator+(UnitValue & value)
{
	UnitExpression exp = UnitExpression();
	exp._left = new UnitExpression(*this);
	exp._operation = Operation::ADDITION;
	exp._right = new UnitExpression(value);
	return exp;
}

rgle::UnitExpression rgle::UnitExpression::operator+(UnitExpression & value)
{
	UnitExpression exp = UnitExpression();
	exp._left = new UnitExpression(*this);
	exp._operation = Operation::ADDITION;
	exp._right = new UnitExpression(value);
	return exp;
}

rgle::UnitExpression rgle::UnitExpression::operator-(UnitValue & value)
{
	UnitExpression exp = UnitExpression();
	exp._left = new UnitExpression(*this);
	exp._operation = Operation::SUBTRACTION;
	exp._right = new UnitExpression(value);
	return exp;
}

rgle::UnitExpression rgle::UnitExpression::operator-(UnitExpression & value)
{
	UnitExpression exp = UnitExpression();
	exp._left = new UnitExpression(*this);
	exp._operation = Operation::SUBTRACTION;
	exp._right = new UnitExpression(value);
	return exp;
}

rgle::UnitExpression rgle::UnitExpression::operator/(UnitValue & value)
{
	UnitExpression exp = UnitExpression();
	exp._left = new UnitExpression(*this);
	exp._operation = Operation::DIVISION;
	exp._right = new UnitExpression(value);
	return exp;
}

rgle::UnitExpression rgle::UnitExpression::operator/(UnitExpression & value)
{
	UnitExpression exp = UnitExpression();
	exp._left = new UnitExpression(*this);
	exp._operation = Operation::DIVISION;
	exp._right = new UnitExpression(value);
	return exp;
}

rgle::UnitExpression rgle::UnitExpression::operator/(float value)
{
	return this->operator*(UnitValue{ value, Unit::ND });
}

rgle::UnitExpression rgle::UnitExpression::operator*(UnitValue & value)
{
	UnitExpression exp = UnitExpression();
	exp._left = new UnitExpression(*this);
	exp._operation = Operation::MULTIPLICATION;
	exp._right = new UnitExpression(value);
	return exp;
}

rgle::UnitExpression rgle::UnitExpression::operator*(UnitExpression & value)
{
	UnitExpression mult = UnitExpression();
	mult._left = new UnitExpression(*this);
	mult._operation = Operation::MULTIPLICATION;
	mult._right = new UnitExpression(value);
	return mult;
}

rgle::UnitExpression rgle::UnitExpression::operator*(float value)
{
	return this->operator*(UnitValue{ value, Unit::ND });
}

void rgle::UnitExpression::operator+=(UnitValue & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::ADDITION;
	this->_right = new UnitExpression(value);
}

void rgle::UnitExpression::operator+=(UnitExpression & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::ADDITION;
	this->_right = new UnitExpression(value);
}

void rgle::UnitExpression::operator-=(UnitValue & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::SUBTRACTION;
	this->_right = new UnitExpression(value);
}

void rgle::UnitExpression::operator-=(UnitExpression & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::SUBTRACTION;
	this->_right = new UnitExpression(value);
}

void rgle::UnitExpression::operator/=(UnitValue & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::DIVISION;
	this->_right = new UnitExpression(value);
}

void rgle::UnitExpression::operator/=(UnitExpression & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::DIVISION;
	this->_right = new UnitExpression(value);
}

void rgle::UnitExpression::operator*=(UnitValue & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::MULTIPLICATION;
	this->_right = new UnitExpression(value);
}

void rgle::UnitExpression::operator*=(UnitExpression & value)
{
	UnitExpression* exp = new UnitExpression();
	exp->_value = this->_value;
	exp->_left = this->_left;
	exp->_operation = this->_operation;
	exp->_right = this->_right;
	this->_value = UnitValue{};
	this->_left = exp;
	this->_operation = Operation::MULTIPLICATION;
	this->_right = new UnitExpression(value);
}

bool rgle::UnitExpression::lessThan(UnitExpression & other, std::shared_ptr<Window> window)
{
	return this->resolve(window) <= other.resolve(window);
}

bool rgle::UnitExpression::greaterThan(UnitExpression & other, std::shared_ptr<Window> window)
{
	return this->resolve(window) >= other.resolve(window);
}

bool rgle::UnitExpression::isZero()
{
	if (this->_operation == Operation::VALUE) {
		if (this->_value.value == 0.0f) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		bool left = this->_left->isZero();
		bool right = this->_right->isZero();
		switch (this->_operation) {
		default:
		case Operation::ADDITION:
		case Operation::SUBTRACTION:
			return left && right;
		case Operation::MULTIPLICATION:
			return left || right;
		case Operation::DIVISION:
			return left;
		}
	}
}

bool rgle::UnitExpression::isValue()
{
	return this->_operation == Operation::VALUE;
}

float rgle::UnitExpression::resolvePixelValue(std::shared_ptr<Window> window, Window::Direction direction)
{
	return 0.0f;
}

float rgle::UnitExpression::resolve(std::shared_ptr<Window> window, Window::Direction direction)
{
	if (this->isValue()) {
		return this->_value.resolve(window, direction);
	}
	switch (this->_operation) {
	case Operation::ADDITION:
		return this->_left->resolve(window, direction) + this->_right->resolve(window, direction);
		break;
	case Operation::SUBTRACTION:
		return this->_left->resolve(window, direction) - this->_right->resolve(window, direction);
		break;
	case Operation::MULTIPLICATION:
		return this->_left->resolve(window, direction) * this->_right->resolve(window, direction);
		break;
	case Operation::DIVISION:
		return this->_left->resolve(window, direction) / this->_right->resolve(window, direction);
		break;
	case Operation::VALUE:
		return this->_left->resolve(window, direction);
		break;
	default:
		throw Exception("invalid operation type", LOGGER_DETAIL_DEFAULT);
	}
}

rgle::UnitVector2D::UnitVector2D()
{
}

rgle::UnitVector2D::UnitVector2D(float x, float y, Unit unit)
{
	this->x = UnitValue{ x, unit };
	this->y = UnitValue{ y, unit };
}

rgle::UnitVector2D::UnitVector2D(UnitExpression & x, UnitExpression & y)
{
	this->x = x;
	this->y = y;
}

rgle::UnitVector2D & rgle::UnitVector2D::parse(std::string parse)
{
	return UnitVector2D();
}

void rgle::UnitVector2D::operator+=(UnitVector2D & other)
{
	this->x += other.x;
	this->x += other.y;
}

glm::vec2 rgle::UnitVector2D::resolve(std::shared_ptr<Window> window)
{
	return glm::vec2(this->x.resolve(window, Window::Direction::X), this->y.resolve(window, Window::Direction::Y));
}

rgle::UnitVector3D::UnitVector3D()
{
}

rgle::UnitVector3D::UnitVector3D(float x, float y, float z, Unit unit)
{
	this->x = UnitValue{ x, unit };
	this->y = UnitValue{ y, unit };
	this->z = UnitValue{ z, unit };
}

rgle::UnitVector3D & rgle::UnitVector3D::parse(std::string parse)
{
	return UnitVector3D();
}

glm::vec3 rgle::UnitVector3D::resolve(std::shared_ptr<Window> window)
{
	return glm::vec3(this->x.resolve(window, Window::Direction::X), this->y.resolve(window, Window::Direction::Y), this->z.resolve(window, Window::Direction::Y));
}

rgle::MouseStateMessage::MouseStateMessage()
{
}

rgle::MouseStateMessage::MouseStateMessage(MouseState state)
{
	this->mouse = state;
}

rgle::MouseStateMessage::~MouseStateMessage()
{
}
