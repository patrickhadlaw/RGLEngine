#pragma once

#include "Renderable.h"
#include "Font.h"
#include "Raycast.h"
#include "Thread.h"

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