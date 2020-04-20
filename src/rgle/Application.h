#pragma once

#include "rgle/Renderable.h"
#include "rgle/Font.h"
#include "rgle/Raycast.h"
#include "rgle/Thread.h"

namespace rgle {

	void initialize();

	class Application : public ContextManager {
	public:
		Application();
		Application(std::string app, std::shared_ptr<Window> window);
		virtual ~Application();

		virtual void initialize();

		virtual std::shared_ptr<ContextManager> getContextManager(std::string managerid);

	protected:
		std::vector<std::shared_ptr<ContextManager>> _contexts;
	};
}