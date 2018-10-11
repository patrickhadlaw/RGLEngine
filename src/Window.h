#pragma once

#include "Event.h"
#include "Graphics.h"

namespace cppogl {

	namespace Conversions {
		const float IN_PER_M = 39.3701f;
	}

	class Context;

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

	class MouseRaycastMessage : public EventMessage {
	public:
		MouseRaycastMessage();
		virtual ~MouseRaycastMessage();

		struct {
			bool hover;
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
	struct UnitVector2D;
	struct UnitVector3D;

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
		friend class Context;
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

		float pixelValue(float value, Unit unit);
		float pixelValue(UnitValue value);

		float parse(UnitValue value, Direction direction = X);
		glm::vec2 parse(UnitVector2D vector, Direction direction = X);
		glm::vec3 parse(UnitVector3D vector, Direction direction = X);

		float convert(float value, Unit from, Unit to);
		UnitValue convert(UnitValue value, Unit to);
		UnitVector2D convert(UnitVector2D vector, Unit to);
		UnitVector3D convert(UnitVector3D vector, Unit to);

		float normalizeX(float value);
		float normalizeY(float value);

		int getKey(int key);
		int getMouseButton(int key);

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
		static std::map<GLFWwindow*, Window*> _handles;
	};
	typedef std::shared_ptr<Window> sWindow;

	struct UnitValue {
		static UnitValue& parse(std::string parse);
		float value = 0.0f;
		Unit unit = Unit::ND;
	};

	struct UnitVector2D {
		float x;
		float y;
		Unit unit = Unit::ND;
	};
	struct UnitVector3D {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		Unit unit = Unit::ND;
	};
}
