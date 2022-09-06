#include "rgle/gfx/Renderable.h"


std::mutex rgle::gfx::ContextManager::_currentContextMutex;
std::thread::id rgle::gfx::ContextManager::_contextThread;
bool rgle::gfx::ContextManager::_contextBound = false;
std::condition_variable rgle::gfx::ContextManager::_contextCondition;
rgle::gfx::Context rgle::gfx::ContextManager::_currentContext;

rgle::gfx::Renderable::Renderable() : _context(ContextManager::getCurrentContext())
{
}

rgle::gfx::Renderable::~Renderable()
{
}

void rgle::gfx::Renderable::update()
{
}

void rgle::gfx::Renderable::render()
{
}

const char * rgle::gfx::Renderable::typeName() const
{
	return "rgle::gfx::Renderable";
}

rgle::gfx::Context & rgle::gfx::Renderable::context()
{
	RGLE_DEBUG_ASSERT(!this->_context.id.empty())
	return this->_context;
}

const rgle::gfx::Context & rgle::gfx::Renderable::context() const
{
	return this->_context;
}

std::weak_ptr<rgle::gfx::ShaderProgram>& rgle::gfx::Renderable::shader()
{
	return this->_shader;
}

const std::weak_ptr<rgle::gfx::ShaderProgram>& rgle::gfx::Renderable::shader() const
{
	return this->_shader;
}

std::shared_ptr<rgle::gfx::ShaderProgram> rgle::gfx::Renderable::shaderLocked() const
{
	if (this->_shader.expired()) {
		throw NullPointerException(LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	return std::move(this->_shader.lock());
}

rgle::gfx::RenderLayer::RenderLayer(
	std::string id,
	std::shared_ptr<ViewTransformer> transformer,
	std::shared_ptr<Viewport> viewport
) :
	_transformer(transformer),
	_viewport(viewport)
{
	this->id = id;
}

rgle::gfx::RenderLayer::RenderLayer(std::string id, std::shared_ptr<ViewTransformer> transformer) :
	RenderLayer(id, transformer, std::make_shared<Viewport>())
{
}

rgle::gfx::RenderLayer::RenderLayer(std::string id) : RenderLayer(id, std::make_shared<ViewTransformer>(), std::make_shared<Viewport>())
{
}

rgle::gfx::RenderLayer::~RenderLayer()
{
}

void rgle::gfx::RenderLayer::update()
{
	clock_t time = clock();
	this->transformer()->update(((float)time - (float)_previousTime) / CLOCKS_PER_SEC);
	this->_previousTime = time;
}

void rgle::gfx::RenderLayer::render()
{
}

void rgle::gfx::RenderLayer::addRenderable(std::shared_ptr<Renderable> renderable)
{
}

const char * rgle::gfx::RenderLayer::typeName() const
{
	return "rgle::gfx::RenderLayer";
}

std::shared_ptr<rgle::gfx::ViewTransformer>& rgle::gfx::RenderLayer::transformer()
{
	return this->_transformer;
}

const std::shared_ptr<rgle::gfx::ViewTransformer>& rgle::gfx::RenderLayer::transformer() const
{
	return this->_transformer;
}

std::shared_ptr<rgle::gfx::Viewport>& rgle::gfx::RenderLayer::viewport()
{
	return this->_viewport;
}

const std::shared_ptr<rgle::gfx::Viewport>& rgle::gfx::RenderLayer::viewport() const
{
	return this->_viewport;
}

rgle::gfx::ContextManager::ContextManager()
{
}

rgle::gfx::ContextManager::ContextManager(std::shared_ptr<Window> window, std::string id)
{
	this->_window = window;
	this->id = id;
	this->_resourceManager = std::make_shared<ResourceManager>();
	this->_shaderManager = std::make_shared<ShaderManager>();
}

rgle::gfx::ContextManager::~ContextManager()
{
}

rgle::gfx::Context rgle::gfx::ContextManager::getContext()
{
	Context context;
	context.id = this->id;
	context.window = this->_window;
	context.manager.shader = _shaderManager;
	context.manager.resource = _resourceManager;
	return context;
}

std::shared_ptr<rgle::gfx::RenderLayer> rgle::gfx::ContextManager::getLayerReference(std::string layer)
{
	for (size_t i = 0; i < _layers.size(); i++) {
		if (this->_layers[i]->id == layer) {
			return this->_layers[i];
		}
	}
	return nullptr;
}

void rgle::gfx::ContextManager::addShader(std::shared_ptr<ShaderProgram> shader)
{
	this->_shaderManager->addShader(shader);
}

void rgle::gfx::ContextManager::addLayer(std::shared_ptr<RenderLayer> layer)
{
	for (size_t i = 0; i < this->_layers.size(); i++) {
		if (this->_layers[i]->id == layer->id) {
			throw IdentifierException("render layer already found", layer->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	this->_layers.push_back(layer);
}

void rgle::gfx::ContextManager::addRenderable(std::string layer, std::shared_ptr<Renderable> renderable)
{
	for (size_t i = 0; i < this->_layers.size(); i++) {
		if (this->_layers[i]->id == layer) {
			this->_layers[i]->addRenderable(renderable);
			return;
		}
	}
	throw IdentifierException("could not find identifier", layer, LOGGER_DETAIL_DEFAULT);
}

void rgle::gfx::ContextManager::addResource(std::shared_ptr<Resource> resource)
{
	this->_resourceManager->addResource(resource);
}

void rgle::gfx::ContextManager::update()
{
	for (size_t i = 0; i < this->_layers.size(); i++) {
		this->_layers[i]->update();
	}
	this->_window->update();
}

void rgle::gfx::ContextManager::render()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Logger::warn("incomplete framebuffer!", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	for (size_t i = 0; i < this->_layers.size(); i++) {
		this->_layers[i]->render();
	}
}

const char * rgle::gfx::ContextManager::typeName() const
{
	return "rgle::gfx::ContextManager";
}

rgle::gfx::Context rgle::gfx::ContextManager::getCurrentContext()
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

void rgle::gfx::ContextManager::executeInContext(std::function<void()> func)
{
	ContextManager::_executeInContext(func, this->getContext());
}

void rgle::gfx::ContextManager::_executeInContext(std::function<void()> func, const Context & context)
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

rgle::gfx::RenderableLayer::RenderableLayer(std::string id) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = std::make_shared<ViewTransformer>();
	_viewport = std::make_shared<Viewport>();
}

rgle::gfx::RenderableLayer::RenderableLayer(std::string id, std::shared_ptr<ViewTransformer> transformer) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = transformer;
	_viewport = std::make_shared<Viewport>(Viewport());
}

rgle::gfx::RenderableLayer::RenderableLayer(std::string id, std::shared_ptr<ViewTransformer> transformer, std::shared_ptr<Viewport> viewport) : RenderLayer(id)
{
	_previousTime = clock();
	this->id = id;
	_transformer = transformer;
	_viewport = viewport;
}

rgle::gfx::RenderableLayer::~RenderableLayer()
{
}

float rgle::gfx::RenderableLayer::getFrameDelay()
{
	return ((float)clock() - (float)this->_previousTime) / CLOCKS_PER_SEC;
}

void rgle::gfx::RenderableLayer::update()
{
	RenderLayer::update();
	for (size_t i = 0; i < this->_renderables.size(); i++) {
		this->_renderables[i]->update();
	}
}

void rgle::gfx::RenderableLayer::render()
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

void rgle::gfx::RenderableLayer::addRenderable(std::shared_ptr<Renderable> renderable)
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

const char * rgle::gfx::RenderableLayer::typeName() const
{
	return "rgle::gfx::RenderableLayer";
}

rgle::gfx::RenderException::RenderException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::gfx::RenderException")
{
}

void rgle::gfx::Context::executeInContext(std::function<void()> func)
{
	ContextManager::_executeInContext(func, *this);
}
