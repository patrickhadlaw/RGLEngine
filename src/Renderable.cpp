#include "Renderable.h"


rgle::Renderable::Renderable()
{
}

rgle::Renderable::Renderable(const Context & context)
{
	this->_context = context;
}

rgle::Renderable::~Renderable()
{
}

void rgle::Renderable::update()
{
}

void rgle::Renderable::render()
{
}

std::string & rgle::Renderable::typeName()
{
	return std::string("rgle::Renderable");
}

rgle::Context & rgle::Renderable::getContext()
{
	return _context;
}

void rgle::Renderable::setContext(Context & context)
{
	this->_context = context;
}

rgle::RenderLayer::RenderLayer(std::string id)
{
	this->id = id;
}

rgle::RenderLayer::~RenderLayer()
{
}

void rgle::RenderLayer::update()
{
}

void rgle::RenderLayer::render()
{
}

void rgle::RenderLayer::addRenderable(std::shared_ptr<Renderable> renderable)
{
}

std::string & rgle::RenderLayer::typeName()
{
	return std::string("rgle::RenderLayer");
}

rgle::ContextManager::ContextManager()
{
}

rgle::ContextManager::ContextManager(std::shared_ptr<Window> window, std::string id)
{
	this->_window = window;
	this->id = id;
	this->_resourceManager = std::shared_ptr<ResourceManager>(new ResourceManager());
	this->_shaderManager = std::shared_ptr<ShaderManager>(new ShaderManager());
}

rgle::ContextManager::~ContextManager()
{
}

rgle::Context rgle::ContextManager::getContext()
{
	Context context;
	context.window = this->_window;
	context.manager.shader = _shaderManager;
	context.manager.resource = _resourceManager;
	return context;
}

std::shared_ptr<rgle::RenderLayer> rgle::ContextManager::getLayerReference(std::string layer)
{
	for (int i = 0; i < _layers.size(); i++) {
		if (_layers[i]->id == layer) {
			return _layers[i];
		}
	}
	return nullptr;
}

void rgle::ContextManager::addShader(std::shared_ptr<ShaderProgram> shader)
{
	this->_shaderManager->addShader(shader);
}

void rgle::ContextManager::addLayer(std::shared_ptr<RenderLayer> layer)
{
	for (int i = 0; i < this->_layers.size(); i++) {
		if (_layers[i]->id == layer->id) {
			throw IdentifierException("render layer already found", layer->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	this->_layers.push_back(layer);
}

void rgle::ContextManager::addRenderable(std::string layer, std::shared_ptr<Renderable> renderable)
{
	for (int i = 0; i < _layers.size(); i++) {
		if (_layers[i]->id == layer) {
			_layers[i]->addRenderable(renderable);
			return;
		}
	}
	throw IdentifierException("could not find identifier", layer, LOGGER_DETAIL_DEFAULT);
}

void rgle::ContextManager::addResource(std::shared_ptr<Resource> resource)
{
	this->_resourceManager->addResource(resource);
}

void rgle::ContextManager::update()
{
	for (int i = 0; i < _layers.size(); i++) {
		_layers[i]->update();
	}
	this->_window->update();
}

void rgle::ContextManager::render()
{
	for (int i = 0; i < _layers.size(); i++) {
		_layers[i]->render();
	}
}

std::string & rgle::ContextManager::typeName()
{
	return std::string("rgle::ContextManager");
}

rgle::RenderableLayer::RenderableLayer(std::string id) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = std::make_shared<ViewTransformer>(ViewTransformer());
	_viewport = std::make_shared<Viewport>(Viewport());
}

rgle::RenderableLayer::RenderableLayer(std::string id, std::shared_ptr<ViewTransformer> transformer) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = transformer;
	_viewport = std::make_shared<Viewport>(Viewport());
}

rgle::RenderableLayer::RenderableLayer(std::string id, std::shared_ptr<ViewTransformer> transformer, std::shared_ptr<Viewport> viewport) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = transformer;
	_viewport = viewport;
}

rgle::RenderableLayer::~RenderableLayer()
{
}

float rgle::RenderableLayer::getFrameDelay()
{
	return ((float)clock() - (float)this->_previousTime) / CLOCKS_PER_SEC;
}

void rgle::RenderableLayer::update()
{
	clock_t time = clock();
	_transformer->update(((float) time - (float) _previousTime) / CLOCKS_PER_SEC);
	_previousTime = time;
	for (int i = 0; i < _renderables.size(); i++) {
		_renderables[i]->update();
	}
}

void rgle::RenderableLayer::render()
{
	_viewport->use();
	GLuint currentShader = 0;
	for (int i = 0; i < _renderables.size(); i++) {
		if (_renderables[i]->shader == nullptr) {
			throw RenderException("failed to render object, shader is null", LOGGER_DETAIL_IDENTIFIER(_renderables[i]->id));
		}
		if (_renderables[i]->shader->programId() != currentShader) {
			currentShader = _renderables[i]->shader->programId();
			_renderables[i]->shader->use();
		}
		_transformer->bind(_renderables[i]->shader);
		_renderables[i]->render();
	}
}

void rgle::RenderableLayer::addRenderable(std::shared_ptr<Renderable> renderable)
{
	if (renderable->shader == nullptr) {
		throw ApplicationException("failed to add renderable to layer, shader is null", LOGGER_DETAIL_IDENTIFIER(renderable->id));
	}
	for (int i = 0; i < _renderables.size(); i++) {
		if (_renderables[i]->id == renderable->id) {
			throw IdentifierException("identifier already exists", renderable->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	for (int i = 0; i < _renderables.size(); i++) {
		if (_renderables[i]->shader->programId() == renderable->shader->programId()) {
			_renderables.insert(_renderables.begin() + i, renderable);
			return;
		}
	}
	this->_renderables.push_back(renderable);
}

std::string & rgle::RenderableLayer::typeName()
{
	return std::string("rgle::RenderableLayer");
}

rgle::RenderException::RenderException(std::string exception, Logger::Detail detail) : Exception(exception, detail)
{
}

rgle::RenderException::~RenderException()
{
}

std::string rgle::RenderException::_type()
{
	return std::string("rgle::RenderException");
}
