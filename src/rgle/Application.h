#pragma once

#include "rgle/gfx/Renderable.h"
#include "rgle/ui/Text.h"
#include "rgle/ray/Raycast.h"
#include "rgle/sync/Thread.h"

namespace rgle {

	void initialize();

	class Application : public gfx::ContextManager {
	public:
		Application();
		Application(std::string app, std::shared_ptr<Window> window);
		virtual ~Application();

		virtual void initialize();

		virtual std::shared_ptr<gfx::ContextManager> getContextManager(std::string managerid);

	protected:
		std::vector<std::shared_ptr<gfx::ContextManager>> _contexts;
	};
}