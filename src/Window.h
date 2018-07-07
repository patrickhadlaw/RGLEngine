#ifndef WINDOW_H
#define WINDOW_H

#include "Graphics.h"

namespace cppogl {
	class Window {
	public:
		Window();
		Window(const int width, const int height, const char* title = "cppogl");
		~Window();
		
		int width();
		int height();

		GLFWwindow* window;
	};
	typedef std::shared_ptr<Window> sWindow;
}

#endif // WINDOW_H