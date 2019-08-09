#pragma once

#include "ShaderProgram.h"
#include "Event.h"

namespace cppogl {

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

	class Window : public EventHost {
		friend struct Context;
	public:

		enum Direction {
			X,
			Y
		};

		Window();
		Window(const int width, const int height, const char* title = "cppogl");
		Window(const Window& other);
		Window(Window&& rvalue);
		~Window();

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
	};

	typedef std::shared_ptr<Window> sWindow;

	struct UnitValue {
		static UnitValue& parse(std::string parse);
		float value = 0.0f;
		Unit unit = Unit::ND;

		float resolvePixelValue(sWindow window, Window::Direction direction = Window::Direction::X);
		float resolve(sWindow window, Window::Direction direction = Window::Direction::X);
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

		void operator=(UnitValue& value);
		UnitExpression operator+(UnitValue& value);
		UnitExpression operator+(UnitExpression& value);
		UnitExpression operator-(UnitValue& value);
		UnitExpression operator-(UnitExpression& value);
		UnitExpression operator/(UnitValue& value);
		UnitExpression operator/(UnitExpression& value);
		UnitExpression operator/(float value);
		UnitExpression operator*(UnitValue& value);
		UnitExpression operator*(UnitExpression& value);
		UnitExpression operator*(float value);
		void operator+=(UnitValue& value);
		void operator+=(UnitExpression& value);
		void operator-=(UnitValue& value);
		void operator-=(UnitExpression& value);
		void operator/=(UnitValue& value);
		void operator/=(UnitExpression& value);
		void operator*=(UnitValue& value);
		void operator*=(UnitExpression& value);

		bool lessThan(UnitExpression& other, sWindow window);
		bool greaterThan(UnitExpression& other, sWindow window);
		bool isZero();
		bool isValue();

		float resolvePixelValue(sWindow window, Window::Direction direction = Window::Direction::X);
		float resolve(sWindow window, Window::Direction direction = Window::Direction::X);

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
		UnitVector2D(UnitExpression& x, UnitExpression& y);
		static UnitVector2D& parse(std::string parse);

		void operator+=(UnitVector2D& other);

		UnitExpression x;
		UnitExpression y;
		glm::vec2 resolve(sWindow window);
	};
	class UnitVector3D {
	public:
		UnitVector3D();
		UnitVector3D(float x, float y, float z, Unit unit = Unit::ND);
		static UnitVector3D& parse(std::string parse);
		UnitExpression x;
		UnitExpression y;
		UnitExpression z;
		glm::vec3 resolve(sWindow window);
	};
}
