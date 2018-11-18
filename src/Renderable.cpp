#include "Renderable.h"


cppogl::Renderable::Renderable()
{
}

cppogl::Renderable::Renderable(const Context & context)
{
	this->_context = context;
}

cppogl::Renderable::~Renderable()
{
}

void cppogl::Renderable::update()
{
}

void cppogl::Renderable::render()
{
}

std::string & cppogl::Renderable::typeName()
{
	return std::string("cppogl::Renderable");
}

cppogl::Context & cppogl::Renderable::getContext()
{
	return _context;
}

void cppogl::Renderable::setContext(Context & context)
{
	this->_context = context;
}

cppogl::RenderLayer::RenderLayer(std::string id)
{
	this->id = id;
}

cppogl::RenderLayer::~RenderLayer()
{
}

void cppogl::RenderLayer::update()
{
}

void cppogl::RenderLayer::render()
{
}

void cppogl::RenderLayer::addRenderable(sRenderable renderable)
{
}

std::string & cppogl::RenderLayer::typeName()
{
	return std::string("cppogl::RenderLayer");
}

cppogl::ContextManager::ContextManager()
{
}

cppogl::ContextManager::ContextManager(sWindow window, std::string id)
{
	this->_window = window;
	this->id = id;
	this->_resourceManager = sResourceManager(new ResourceManager());
	this->_shaderManager = sShaderManager(new ShaderManager());
}

cppogl::ContextManager::~ContextManager()
{
}

cppogl::Context cppogl::ContextManager::getContext()
{
	Context context;
	context.window = this->_window;
	context.manager.shader = _shaderManager;
	context.manager.resource = _resourceManager;
	return context;
}

cppogl::sRenderLayer cppogl::ContextManager::getLayerReference(std::string layer)
{
	for (int i = 0; i < _layers.size(); i++) {
		if (_layers[i]->id == layer) {
			return _layers[i];
		}
	}
	return nullptr;
}

void cppogl::ContextManager::addShader(sShaderProgram shader)
{
	this->_shaderManager->addShader(shader);
}

void cppogl::ContextManager::addLayer(sRenderLayer layer)
{
	for (int i = 0; i < this->_layers.size(); i++) {
		if (_layers[i]->id == layer->id) {
			throw IdentifierException("render layer already found", layer->id, EXCEPT_DETAIL_DEFAULT);
		}
	}
	this->_layers.push_back(layer);
}

void cppogl::ContextManager::addRenderable(std::string layer, sRenderable renderable)
{
	for (int i = 0; i < _layers.size(); i++) {
		if (_layers[i]->id == layer) {
			_layers[i]->addRenderable(renderable);
			return;
		}
	}
	throw IdentifierException("could not find identifier", layer, EXCEPT_DETAIL_DEFAULT);
}

void cppogl::ContextManager::addResource(sResource resource)
{
	this->_resourceManager->addResource(resource);
}

void cppogl::ContextManager::update()
{
	for (int i = 0; i < _layers.size(); i++) {
		_layers[i]->update();
	}
	this->_window->update();
}

void cppogl::ContextManager::render()
{
	for (int i = 0; i < _layers.size(); i++) {
		_layers[i]->render();
	}
}

std::string & cppogl::ContextManager::typeName()
{
	return std::string("cppogl::ContextManager");
}

cppogl::RenderableLayer::RenderableLayer(std::string id) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = std::make_shared<ViewTransformer>(ViewTransformer());
	_viewport = std::make_shared<Viewport>(Viewport());
}

cppogl::RenderableLayer::RenderableLayer(std::string id, sViewTransformer transformer) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = transformer;
	_viewport = std::make_shared<Viewport>(Viewport());
}

cppogl::RenderableLayer::RenderableLayer(std::string id, sViewTransformer transformer, sViewport viewport) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = transformer;
	_viewport = viewport;
}

cppogl::RenderableLayer::~RenderableLayer()
{
}

float cppogl::RenderableLayer::getFrameDelay()
{
	return ((float)clock() - (float)this->_previousTime) / CLOCKS_PER_SEC;
}

void cppogl::RenderableLayer::update()
{
	clock_t time = clock();
	_transformer->update(((float) time - (float) _previousTime) / CLOCKS_PER_SEC);
	_previousTime = time;
	for (int i = 0; i < _renderables.size(); i++) {
		_renderables[i]->update();
	}
}

void cppogl::RenderableLayer::render()
{
	_viewport->use();
	GLuint currentShader = 0;
	for (int i = 0; i < _renderables.size(); i++) {
		if (_renderables[i]->shader == nullptr) {
			throw RenderException("failed to render object, shader is null", EXCEPT_DETAIL_IDENTIFIER(_renderables[i]->id));
		}
		if (_renderables[i]->shader->programId() != currentShader) {
			currentShader = _renderables[i]->shader->programId();
			_renderables[i]->shader->use();
		}
		_transformer->bind(_renderables[i]->shader);
		_renderables[i]->render();
	}
}

void cppogl::RenderableLayer::addRenderable(sRenderable renderable)
{
	if (renderable->shader == nullptr) {
		throw ApplicationException("failed to add renderable to layer, shader is null", EXCEPT_DETAIL_IDENTIFIER(renderable->id));
	}
	for (int i = 0; i < _renderables.size(); i++) {
		if (_renderables[i]->id == renderable->id) {
			throw IdentifierException("identifier already exists", renderable->id, EXCEPT_DETAIL_DEFAULT);
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

std::string & cppogl::RenderableLayer::typeName()
{
	return std::string("cppogl::RenderableLayer");
}

cppogl::RenderException::RenderException(std::string exception, Exception::Detail detail) : Exception(exception, detail)
{
}

cppogl::RenderException::~RenderException()
{
}

std::string cppogl::RenderException::_type()
{
	return std::string("cppogl::RenderException");
}
