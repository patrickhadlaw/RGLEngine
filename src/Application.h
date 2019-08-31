#pragma once

#include "Renderable.h"
#include "Font.h"
#include "Raycast.h"
#include "Thread.h"

namespace rgle {

	void initialize();

	class Application : public ContextManager {
	public:
		Application();
		Application(std::string app, std::shared_ptr<Window> window);
		virtual ~Application();

		virtual void initialize();

		virtual std::shared_ptr<ContextManager> getContextManager(std::string id);

	protected:
		std::vector<std::shared_ptr<ContextManager>> _contexts;
	};
}