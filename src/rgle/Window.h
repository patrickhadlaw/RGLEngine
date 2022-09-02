#pragma once

#include "rgle/gfx/ShaderProgram.h"
#include "rgle/Event.h"

namespace rgle {

	namespace Conversions {
		const float IN_PER_M = 39.3701f;
	}

	struct Context;

	struct MouseState {
		double x = 0.0;
		double y = 0.0;
		int button;
		int action;
		int modifier;
	};

	class MouseStateMessage : public EventMessage {
	public:
		MouseStateMessage();
		MouseStateMessage(MouseState state);
		virtual ~MouseStateMessage();
		
		MouseState mouse;
	};

	class MouseMoveMessage : public EventMessage {
	public:
		MouseMoveMessage();
		virtual ~MouseMoveMessage();

		struct {
			double x = 0.0;
			double y = 0.0;
		} mouse;
	};

	class MouseClickMessage : public EventMessage {
	public:
		MouseClickMessage();
		virtual ~MouseClickMessage();

		struct {
			int button;
			int action;
			int modifier;
		} mouse;
	};

	class WindowResizeMessage : public EventMessage {
	public:
		WindowResizeMessage();
		virtual ~WindowResizeMessage();

		struct {
			int width;
			int height;
		} window;
	};

	class KeyboardMessage : public EventMessage {
	public:
		KeyboardMessage();
		virtual ~KeyboardMessage();

		struct {
			int key;
			int scancode;
			int action;
			int mode;
		} keyboard;
	};

	struct UnitValue;
	class UnitVector2D;
	class UnitVector3D;

	enum class Unit {
		PX,		// Pixel
		ND,		// Normalized device coordinate
		PT,		// Point -- 1/72 of inch
		VW,		// Percent of viewport width
		VH,		// Percetn of viewport height
		WW,		// Percent of window width
		WH,		// Percent of window height
		IN,		// Inch of screen
		CM,		// Centimetre of screen
		ERROR	// Error unit, failed to parse unit
	};
	static Unit unitFromPostfix(std::string string);
	static std::string postfixFromUnit(Unit unit);

	void glfw_error_callback(int code, const char* message);

	class Window : public EventHost, public Node {
		friend struct Context;
	public:

		enum Direction {
			X,
			Y
		};

		Window(const int width, const int height, const char* title = "rgle");
		Window(const Window&) = delete;
		Window(Window&& rvalue);
		~Window();

		void operator=(const Window&) = delete;
		void operator=(Window&& rvalue);
		
		int width();
		int height();

		float physicalWidth();
		float physicalHeight();

		float pixelValue(float value, Unit unit, Direction direction = X);
		float pixelValue(UnitValue value, Direction direction = X);

		float resolve(UnitValue value, Direction direction = X);
		glm::vec2 resolve(UnitVector2D vector);
		glm::vec3 resolve(UnitVector3D vector);

		float convert(float value, Unit from, Unit to);
		UnitValue convert(UnitValue value, Unit to);
		UnitVector2D convert(UnitVector2D vector, Unit to);
		UnitVector3D convert(UnitVector3D vector, Unit to);

		float pointValue(float convert, Direction direction = X);

		float normalizeX(float value);
		float normalizeY(float value);

		int getKey(int key);
		int getMouseButton(int key);
		glm::vec2 getCursorPosition();

		void setInputMode(int mode, int value);
		void setCursor(int type);

		void grabCursor();
		void ungrabCursor();

		bool grabbed();

		bool shouldClose();

		void update();

		static void handleEvent(GLFWwindow* handle, std::string eventname, EventMessage* message);

	private:
		bool _grabbed;
		struct {
			double x;
			double y;
		} _initial;
		GLFWwindow* _window;
		GLFWcursor* _cursor;
		static std::map<GLFWwindow*, Window*> _handles;
		static bool _initialized;
	};

	struct UnitValue {
		static UnitValue parse(std::string parse);
		float value = 0.0f;
		Unit unit = Unit::ND;

		float resolvePixelValue(std::shared_ptr<Window> window, Window::Direction direction = Window::Direction::X) const;
		float resolve(std::shared_ptr<Window> window, Window::Direction direction = Window::Direction::X) const;
	};

	enum class Operation {
		VALUE,
		ADDITION,
		SUBTRACTION,
		MULTIPLICATION,
		DIVISION
	};

	class UnitExpression {
	public:
		UnitExpression();
		UnitExpression(UnitValue value);
		UnitExpression(UnitValue left, char op, UnitValue right);
		UnitExpression(UnitValue left, char op, UnitExpression right);
		UnitExpression(UnitExpression left, char op, UnitExpression right);
		UnitExpression(const UnitExpression& other);
		UnitExpression(UnitExpression&& rvalue);
		virtual ~UnitExpression();

		void operator=(const UnitExpression& other);
		void operator=(UnitExpression&& rvalue);

		void operator=(const UnitValue& value);
		UnitExpression operator+(const UnitValue& value) const;
		UnitExpression operator+(const UnitExpression& value) const;
		UnitExpression operator-(const UnitValue& value) const;
		UnitExpression operator-(const UnitExpression& value) const;
		UnitExpression operator/(const UnitValue& value) const;
		UnitExpression operator/(const UnitExpression& value) const;
		UnitExpression operator/(const float& value) const;
		UnitExpression operator*(const UnitValue& value) const;
		UnitExpression operator*(const UnitExpression& value) const;
		UnitExpression operator*(const float& value) const;
		void operator+=(const UnitValue& value);
		void operator+=(const UnitExpression& value);
		void operator-=(const UnitValue& value);
		void operator-=(const UnitExpression& value);
		void operator/=(const UnitValue& value);
		void operator/=(const UnitExpression& value);
		void operator*=(const UnitValue& value);
		void operator*=(const UnitExpression& value);

		bool lessThan(const UnitExpression& other, std::shared_ptr<Window> window) const;
		bool greaterThan(const UnitExpression& other, std::shared_ptr<Window> window) const;
		bool isZero() const;
		bool isValue() const;

		float resolvePixelValue(std::shared_ptr<Window> window, Window::Direction direction = Window::Direction::X) const;
		float resolve(std::shared_ptr<Window> window, Window::Direction direction = Window::Direction::X) const;

	private:

		UnitValue _value = UnitValue{};

		UnitExpression* _left = nullptr;
		Operation _operation = Operation::VALUE;
		UnitExpression* _right = nullptr;
	};

	class UnitVector2D {
	public:
		UnitVector2D();
		UnitVector2D(float x, float y, Unit unit = Unit::ND);
		UnitVector2D(const UnitExpression& x, const UnitExpression& y);
		static UnitVector2D parse(std::string parse);

		void operator+=(UnitVector2D& other);

		UnitExpression x;
		UnitExpression y;
		glm::vec2 resolve(std::shared_ptr<Window> window);
	};
	class UnitVector3D {
	public:
		UnitVector3D();
		UnitVector3D(float x, float y, float z, Unit unit = Unit::ND);
		static UnitVector3D parse(std::string parse);
		UnitExpression x;
		UnitExpression y;
		UnitExpression z;
		glm::vec3 resolve(std::shared_ptr<Window> window);
	};
}
