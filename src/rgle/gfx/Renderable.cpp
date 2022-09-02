#include "rgle/gfx/Renderable.h"


std::mutex rgle::ContextManager::_currentContextMutex;
std::thread::id rgle::ContextManager::_contextThread;
bool rgle::ContextManager::_contextBound = false;
std::condition_variable rgle::ContextManager::_contextCondition;
rgle::Context rgle::ContextManager::_currentContext;

rgle::Renderable::Renderable() : _context(ContextManager::getCurrentContext())
{
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

const char * rgle::Renderable::typeName() const
{
	return "rgle::Renderable";
}

rgle::Context & rgle::Renderable::context()
{
	RGLE_DEBUG_ASSERT(!this->_context.id.empty())
	return this->_context;
}

const rgle::Context & rgle::Renderable::context() const
{
	return this->_context;
}

std::weak_ptr<rgle::ShaderProgram>& rgle::Renderable::shader()
{
	return this->_shader;
}

const std::weak_ptr<rgle::ShaderProgram>& rgle::Renderable::shader() const
{
	return this->_shader;
}

std::shared_ptr<rgle::ShaderProgram> rgle::Renderable::shaderLocked() const
{
	if (this->_shader.expired()) {
		throw NullPointerException(LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	return std::move(this->_shader.lock());
}

rgle::RenderLayer::RenderLayer(
	std::string id,
	std::shared_ptr<ViewTransformer> transformer,
	std::shared_ptr<Viewport> viewport
) :
	_transformer(transformer),
	_viewport(viewport)
{
	this->id = id;
}

rgle::RenderLayer::RenderLayer(std::string id, std::shared_ptr<ViewTransformer> transformer) :
	RenderLayer(id, transformer, std::make_shared<Viewport>())
{
}

rgle::RenderLayer::RenderLayer(std::string id) : RenderLayer(id, std::make_shared<ViewTransformer>(), std::make_shared<Viewport>())
{
}

rgle::RenderLayer::~RenderLayer()
{
}

void rgle::RenderLayer::update()
{
	clock_t time = clock();
	this->transformer()->update(((float)time - (float)_previousTime) / CLOCKS_PER_SEC);
	this->_previousTime = time;
}

void rgle::RenderLayer::render()
{
}

void rgle::RenderLayer::addRenderable(std::shared_ptr<Renderable> renderable)
{
}

const char * rgle::RenderLayer::typeName() const
{
	return "rgle::RenderLayer";
}

std::shared_ptr<rgle::ViewTransformer>& rgle::RenderLayer::transformer()
{
	return this->_transformer;
}

const std::shared_ptr<rgle::ViewTransformer>& rgle::RenderLayer::transformer() const
{
	return this->_transformer;
}

std::shared_ptr<rgle::Viewport>& rgle::RenderLayer::viewport()
{
	return this->_viewport;
}

const std::shared_ptr<rgle::Viewport>& rgle::RenderLayer::viewport() const
{
	return this->_viewport;
}

rgle::ContextManager::ContextManager()
{
}

rgle::ContextManager::ContextManager(std::shared_ptr<Window> window, std::string id)
{
	this->_window = window;
	this->id = id;
	this->_resourceManager = std::make_shared<ResourceManager>();
	this->_shaderManager = std::make_shared<ShaderManager>();
}

rgle::ContextManager::~ContextManager()
{
}

rgle::Context rgle::ContextManager::getContext()
{
	Context context;
	context.id = this->id;
	context.window = this->_window;
	context.manager.shader = _shaderManager;
	context.manager.resource = _resourceManager;
	return context;
}

std::shared_ptr<rgle::RenderLayer> rgle::ContextManager::getLayerReference(std::string layer)
{
	for (size_t i = 0; i < _layers.size(); i++) {
		if (this->_layers[i]->id == layer) {
			return this->_layers[i];
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
	for (size_t i = 0; i < this->_layers.size(); i++) {
		if (this->_layers[i]->id == layer->id) {
			throw IdentifierException("render layer already found", layer->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	this->_layers.push_back(layer);
}

void rgle::ContextManager::addRenderable(std::string layer, std::shared_ptr<Renderable> renderable)
{
	for (size_t i = 0; i < this->_layers.size(); i++) {
		if (this->_layers[i]->id == layer) {
			this->_layers[i]->addRenderable(renderable);
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
	for (size_t i = 0; i < this->_layers.size(); i++) {
		this->_layers[i]->update();
	}
	this->_window->update();
}

void rgle::ContextManager::render()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Logger::warn("incomplete framebuffer!", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	for (size_t i = 0; i < this->_layers.size(); i++) {
		this->_layers[i]->render();
	}
}

const char * rgle::ContextManager::typeName() const
{
	return "rgle::ContextManager";
}

rgle::Context rgle::ContextManager::getCurrentContext()
{
	if (ContextManager::_contextBound) {
		return ContextManager::_currentContext;
	}
	else {
		throw ApplicationException(
			"failed to retrive current context, not running in a context (all contextual execution should be ran through executeInContext)",
			LOGGER_DETAIL_DEFAULT
		);
	}
}

void rgle::ContextManager::executeInContext(std::function<void()> func)
{
	ContextManager::_executeInContext(func, this->getContext());
}

void rgle::ContextManager::_executeInContext(std::function<void()> func, const Context & context)
{
	if (context.id.empty() || context.manager.resource.expired() || context.manager.shader.expired() || context.window.expired()) {
		throw IllegalArgumentException("invalid context", LOGGER_DETAIL_DEFAULT);
	}
	bool subexecution = false;
	{
		std::unique_lock<std::mutex> lk(ContextManager::_currentContextMutex);
		if (ContextManager::_contextBound) {
			if (ContextManager::_contextThread == std::this_thread::get_id()) {
				if (context.id == ContextManager::_currentContext.id) {
					subexecution = true;
				}
				else {
					throw ApplicationException(
						"invalid context switch: " + ContextManager::_currentContext.id + " -> " + context.id,
						LOGGER_DETAIL_DEFAULT
					);
				}
			}
			else {
				ContextManager::_contextCondition.wait(lk, [] { return !ContextManager::_contextBound; });
			}
		}
		if (!subexecution) {
			ContextManager::_contextBound = true;
			ContextManager::_contextThread = std::this_thread::get_id();
			ContextManager::_currentContext = context;
		}
	}
	func();
	if (!subexecution) {
		{
			std::scoped_lock<std::mutex> lk(ContextManager::_currentContextMutex);
			ContextManager::_contextBound = false;
		}
		ContextManager::_contextCondition.notify_one();
	}
}

rgle::RenderableLayer::RenderableLayer(std::string id) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = std::make_shared<ViewTransformer>();
	_viewport = std::make_shared<Viewport>();
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
	RenderLayer::update();
	for (size_t i = 0; i < this->_renderables.size(); i++) {
		this->_renderables[i]->update();
	}
}

void rgle::RenderableLayer::render()
{
	this->_viewport->use();
	GLuint currentShader = 0;
	for (size_t i = 0; i < this->_renderables.size(); i++) {
		auto shader = this->_renderables[i]->shaderLocked();
		if (shader->programId() != currentShader) {
			currentShader = shader->programId();
			shader->use();
		}
		this->_transformer->bind(shader);
		this->_renderables[i]->render();
	}
}

void rgle::RenderableLayer::addRenderable(std::shared_ptr<Renderable> renderable)
{
	if (renderable->shader().expired()) {
		throw ApplicationException("failed to add renderable to layer, shader is null", LOGGER_DETAIL_IDENTIFIER(renderable->id));
	}
	for (size_t i = 0; i < this->_renderables.size(); i++) {
		if (this->_renderables[i]->id == renderable->id) {
			throw IdentifierException("identifier already exists", renderable->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	GLuint programId = renderable->shaderLocked()->programId();
	for (size_t i = 0; i < this->_renderables.size(); i++) {
		if (this->_renderables[i]->shaderLocked()->programId() == programId) {
			this->_renderables.insert(this->_renderables.begin() + i, renderable);
			return;
		}
	}
	this->_renderables.push_back(renderable);
}

const char * rgle::RenderableLayer::typeName() const
{
	return "rgle::RenderableLayer";
}

rgle::RenderException::RenderException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::RenderException")
{
}

void rgle::Context::executeInContext(std::function<void()> func)
{
	ContextManager::_executeInContext(func, *this);
}
