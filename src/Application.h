#pragma once

#include <signal.h>

#include "Renderable.h"

void CPPOGL_AbortHandler(int signal_number);

namespace cppogl {

	class Application : public ContextManager {
	public:
		Application();
		Application(std::string app, sWindow window);
		virtual ~Application();

		virtual void initialize();

		virtual sContextManager getContextManager(std::string id);

	protected:
		std::vector<sContextManager> _contexts;
	};
}