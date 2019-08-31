#pragma once

#include "Window.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Node.h"

namespace rgle {

	class RenderException : public Exception {
	public:
		RenderException(std::string exception, Logger::Detail detail);
		virtual ~RenderException();

	protected:
		virtual std::string _type();
	};

	class Application;
	typedef std::shared_ptr<Application> sApplication;

	struct Context {
		std::shared_ptr<Window> window;
		struct {
			std::shared_ptr<ShaderManager> shader;
			std::shared_ptr<ResourceManager> resource;
		} manager;
	};

	class RenderLayer;

	class Renderable : public Node {
		friend class RenderLayer;
	public:
		Renderable();
		Renderable(const Context& context);
		virtual ~Renderable();

		virtual void update();
		virtual void render();

		virtual std::string& typeName();

		virtual Context& getContext();
		virtual void setContext(Context& context);

		std::shared_ptr<ShaderProgram> shader;
	protected:
		Context _context;
	};

	class RenderLayer : public Renderable {
	public:
		RenderLayer(std::string id);
		virtual ~RenderLayer();

		virtual void update();
		virtual void render();

		virtual void addRenderable(std::shared_ptr<Renderable> renderable);
		
		virtual std::string & typeName();

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

		virtual std::string & typeName();

	protected:
		clock_t _previousTime;
		std::vector<std::shared_ptr<Renderable>> _renderables;
		std::shared_ptr<ViewTransformer> _transformer;
		std::shared_ptr<Viewport> _viewport;
	};


	class ContextManager : public Node {
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

		virtual std::string& typeName();

	protected:
		std::shared_ptr<Window> _window;
		std::shared_ptr<ShaderManager> _shaderManager;
		std::shared_ptr<ResourceManager> _resourceManager = nullptr;
		std::vector<std::shared_ptr<RenderLayer>> _layers;
	};
}