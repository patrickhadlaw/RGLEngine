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

	class Window : public EventHost {
		friend class Context;
	public:

		enum class Unit {
			PT,		// Point -- 1/72 of inch
			VW,		// Percent of viewport width
			VH,		// Percetn of viewport height
			WW,		// Percent of window width
			WH,		// Percent of window height
			IN,		// Inch of screen
			CM		// Centimetre of screen
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

		float parseUnit(float value, Unit unit);
		float parseUnit(std::string value);

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
}
