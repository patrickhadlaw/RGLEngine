#pragma once

#include "Window.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Node.h"

namespace rgle {

	class RenderException : public Exception {
	public:
		RenderException(std::string exception, Exception::Detail detail);
		virtual ~RenderException();

	protected:
		virtual std::string _type();
	};

	class Application;
	typedef std::shared_ptr<Application> sApplication;

	struct Context {
		sWindow window;
		struct {
			sShaderManager shader;
			sResourceManager resource;
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

		sShaderProgram shader;
	protected:
		Context _context;
	};

	typedef std::shared_ptr<Renderable> sRenderable;

	class RenderLayer : public Renderable {
	public:
		RenderLayer(std::string id);
		virtual ~RenderLayer();

		virtual void update();
		virtual void render();

		virtual void addRenderable(sRenderable renderable);
		
		virtual std::string & typeName();

	};

	typedef std::shared_ptr<RenderLayer> sRenderLayer;

	class RenderableLayer : public RenderLayer {
	public:
		RenderableLayer(std::string id);
		RenderableLayer(std::string id, sViewTransformer transformer);
		RenderableLayer(std::string id, sViewTransformer transformer, sViewport viewport);
		virtual ~RenderableLayer();

		float getFrameDelay();

		virtual void update();
		virtual void render();

		virtual void addRenderable(sRenderable renderable);

		virtual std::string & typeName();

	protected:
		clock_t _previousTime;
		std::vector<sRenderable> _renderables;
		sViewTransformer _transformer;
		sViewport _viewport;
	};

	typedef std::shared_ptr<RenderableLayer> sRenderableLayer;

	class ContextManager : public Node {
	public:
		ContextManager();
		ContextManager(sWindow window, std::string id);
		virtual ~ContextManager();

		virtual Context getContext();

		virtual sRenderLayer getLayerReference(std::string layer);

		virtual void addShader(sShaderProgram shader);

		virtual void addLayer(sRenderLayer layer);

		virtual void addRenderable(std::string layer, sRenderable renderable);

		virtual void addResource(sResource resource);

		template<typename Type>
		std::shared_ptr<Type> getResource(std::string id) {
			return this->_resourceManager->getResource<Type>(id);
		}
		
		virtual void update();
		virtual void render();

		virtual std::string& typeName();

	protected:
		sWindow _window;
		sShaderManager _shaderManager;
		sResourceManager _resourceManager = nullptr;
		std::vector<sRenderLayer> _layers;
	};

	typedef std::shared_ptr<ContextManager> sContextManager;
}