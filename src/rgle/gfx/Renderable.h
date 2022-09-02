#pragma once

#include "rgle/Window.h"
#include "rgle/gfx/ShaderProgram.h"
#include "rgle/gfx/Camera.h"
#include "rgle/Node.h"

namespace rgle {

	class RenderException : public Exception {
	public:
		RenderException(std::string exception, Logger::Detail detail);
	};

	class Application;

	struct Context {
		std::string id;
		std::weak_ptr<Window> window;
		struct {
			std::weak_ptr<ShaderManager> shader;
			std::weak_ptr<ResourceManager> resource;
		} manager;

		void executeInContext(std::function<void()> func);
	};

	class RenderLayer;

	class Renderable : public Node {
		friend class RenderLayer;
	public:
		Renderable();
		virtual ~Renderable();

		virtual void update();
		virtual void render();

		virtual const char* typeName() const;

		Context& context();
		const Context& context() const;

		std::weak_ptr<ShaderProgram>& shader();
		const std::weak_ptr<ShaderProgram>& shader() const;

		std::shared_ptr<ShaderProgram> shaderLocked() const;

	private:
		Context _context;
		std::weak_ptr<ShaderProgram> _shader;
	};

	class RenderLayer : public Renderable {
	public:
		RenderLayer(std::string id, std::shared_ptr<ViewTransformer> transformer, std::shared_ptr<Viewport> viewport);
		RenderLayer(std::string id, std::shared_ptr<ViewTransformer> transformer);
		RenderLayer(std::string id);
		virtual ~RenderLayer();

		virtual void update();
		virtual void render();

		virtual void addRenderable(std::shared_ptr<Renderable> renderable);
		
		virtual const char* typeName() const;

		std::shared_ptr<ViewTransformer>& transformer();
		const std::shared_ptr<ViewTransformer>& transformer() const;

		std::shared_ptr<Viewport>& viewport();
		const std::shared_ptr<Viewport>& viewport() const;

	protected:
		clock_t _previousTime;
		std::shared_ptr<ViewTransformer> _transformer;
		std::shared_ptr<Viewport> _viewport;
	};

	class RenderableLayer : public RenderLayer {
	public:
		RenderableLayer(std::string id);
		RenderableLayer(std::string id, std::shared_ptr<ViewTransformer> transformer);
		RenderableLayer(std::string id, std::shared_ptr<ViewTransformer> transformer, std::shared_ptr<Viewport> viewport);
		virtual ~RenderableLayer();

		float getFrameDelay();

		virtual void update();
		virtual void render();

		virtual void addRenderable(std::shared_ptr<Renderable> renderable);

		virtual const char* typeName() const;

	protected:
		std::vector<std::shared_ptr<Renderable>> _renderables;
	};


	class ContextManager : public Node {
		friend struct Context;
	public:
		ContextManager();
		ContextManager(std::shared_ptr<Window> window, std::string id);
		virtual ~ContextManager();

		virtual Context getContext();

		virtual std::shared_ptr<RenderLayer> getLayerReference(std::string layer);

		virtual void addShader(std::shared_ptr<ShaderProgram> shader);

		virtual void addLayer(std::shared_ptr<RenderLayer> layer);

		virtual void addRenderable(std::string layer, std::shared_ptr<Renderable> renderable);

		virtual void addResource(std::shared_ptr<Resource> resource);

		template<typename Type>
		std::shared_ptr<Type> getResource(std::string id) {
			return this->_resourceManager->getResource<Type>(id);
		}
		
		virtual void update();
		virtual void render();

		virtual const char* typeName() const;

		static Context getCurrentContext();
		void executeInContext(std::function<void()> func);

	protected:
		std::shared_ptr<Window> _window;
		std::shared_ptr<ShaderManager> _shaderManager;
		std::shared_ptr<ResourceManager> _resourceManager = nullptr;
		std::vector<std::shared_ptr<RenderLayer>> _layers;
	private:
		static void _executeInContext(std::function<void()> func, const Context& context);

		static std::mutex _currentContextMutex;
		static std::thread::id _contextThread;
		static bool _contextBound;
		static std::condition_variable _contextCondition;
		static Context _currentContext;
	};
}