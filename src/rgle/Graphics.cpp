#include "rgle/Graphics.h"


size_t rgle::InstancedRenderer::_idCounter = 0;

rgle::GraphicsException::GraphicsException(std::string except, Logger::Detail & detail) : Exception(except, detail, "rgle::GraphicsException")
{
}

rgle::Shape::Shape()
{
	this->model.matrix = glm::mat4(1.0f);
}

rgle::Shape::Shape(std::string shaderid, std::vector<glm::vec3> verticies, std::vector<glm::vec4> colors)
{
	this->color.list = colors;
	this->vertex.list = verticies;
	this->shader() = (*this->context().manager.shader.lock())[shaderid];

	model.matrix = glm::mat4(1.0f);
	auto shader = this->shaderLocked();
	model.location = glGetUniformLocation(shader->programId(), "model");
	if (model.location < 0) {
		throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	color.location = glGetAttribLocation(shader->programId(), "vertex_color");
	if (color.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (vertex.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}

	this->generate();
}

rgle::Shape::Shape(std::string shaderid, std::vector<glm::vec3> verticies, glm::vec4 color)
{
	std::vector<glm::vec4> colors;
	colors.resize(verticies.size());
	for (int i = 0; i < colors.size(); i++) {
		colors[i] = color;
	}
	Shape(shaderid, verticies, colors);
}

rgle::Shape::~Shape()
{

}

void rgle::Shape::render()
{
	this->standardRender(this->shaderLocked());
}

const char * rgle::Shape::typeName() const
{
	return "rgle::Shape";
}

void rgle::Shape::translate(float x, float y, float z)
{
	glm::mat4 translate(1.0f);
	translate[3][0] = x;
	translate[3][1] = y;
	translate[3][2] = z;
	model.matrix = model.matrix*translate;
}

void rgle::Shape::rotate(float x, float y, float z)
{
	glm::mat4 rotateX(1.0f);
	rotateX[1][1] = cosf(x);
	rotateX[1][2] = sinf(x);
	rotateX[2][1] = -sinf(x);
	rotateX[2][2] = cosf(x);
	glm::mat4 rotateY(1.0f);
	rotateY[0][0] = cosf(y);
	rotateY[2][0] = sinf(y);
	rotateY[0][2] = -sinf(y);
	rotateY[2][2] = cosf(y);
	glm::mat4 rotateZ(1.0f);
	rotateZ[0][0] = cosf(z);
	rotateZ[0][1] = sinf(z);
	rotateZ[1][0] = -sinf(z);
	rotateZ[1][1] = cosf(z);
	glm::mat4 rotation = rotateX*rotateY*rotateZ;
	model.matrix = rotation*model.matrix;
}

rgle::Sampler2D::Sampler2D()
{
	this->enabled = true;
	this->texture = nullptr;
	this->enableLocation = -1;
	this->samplerLocation = -1;
}

rgle::Sampler2D::Sampler2D(std::weak_ptr<ShaderProgram> shader, std::shared_ptr<Texture> texture, std::string samplerUniform)
{
	this->enabled = true;
	this->shader = shader;
	this->texture = texture;
	this->_generate(samplerUniform);
}

rgle::Sampler2D::Sampler2D(const Sampler2D & other)
{
	this->enabled = other.enabled;
	this->shader = other.shader;
	this->texture = other.texture;
	this->enableLocation = other.enableLocation;
	this->samplerLocation = other.samplerLocation;
}

void rgle::Sampler2D::operator=(const Sampler2D& other)
{
	this->enabled = other.enabled;
	this->shader = other.shader;
	this->texture = other.texture;
	this->enableLocation = other.enableLocation;
	this->samplerLocation = other.samplerLocation;
}

rgle::Sampler2D::~Sampler2D()
{
}

void rgle::Sampler2D::use()
{
	if (this->texture == nullptr || this->shader.expired()) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	if (this->enableLocation > 0) {
		glUniform1i(this->enableLocation, enabled);
	}
	glUniform1i(this->samplerLocation, this->texture->index());
	this->texture->bind();
}

void rgle::Sampler2D::_generate(const std::string& samplerUniform)
{
	auto shader = this->shader.lock();
	this->enableLocation = glGetUniformLocation(shader->programId(), "enable_texture");
	this->samplerLocation = glGetUniformLocation(shader->programId(), samplerUniform.c_str());
	if (this->samplerLocation < 0) {
		throw GraphicsException("failed to get uniform location of: '" + samplerUniform + '\'', LOGGER_DETAIL_DEFAULT);
	}
}

rgle::Triangle::Triangle()
{
}

rgle::Triangle::Triangle(std::string shaderid, float a, float b, float theta, glm::vec4 color)
{	
	this->_construct(shaderid, a, b, theta, std::vector<glm::vec4>({ color, color, color }));
}

rgle::Triangle::Triangle(std::string shaderid, float a, float b, float theta, std::vector<glm::vec4> colors)
{
	this->_construct(shaderid, a, b, theta, colors);
}

void rgle::Triangle::_construct(std::string& shaderid, float& a, float& b, float& theta, std::vector<glm::vec4> colors)
{
	auto shader = (*this->context().manager.shader.lock())[shaderid];
	this->shader() = shader;
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(shader->programId(), "model");
	if (this->model.location < 0) {
		throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	this->color.location = glGetAttribLocation(shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(a, 0.0, 0.0),
		glm::vec3(b * cosf(theta), b * sinf(theta), 0.0)
	};
	if (colors.size() < 3) {
		throw Exception("invalid triangle colors, expected 3 got " + std::to_string(colors.size()), LOGGER_DETAIL_DEFAULT);
	}
	this->color.list = colors;
	this->generate();
}

rgle::Triangle::~Triangle()
{
}

rgle::Rect::Rect()
{
}

rgle::Rect::Rect(std::string shaderid, float width, float height, std::vector<glm::vec4> colors)
{
	this->_construct(shaderid, width, height, colors);
}

rgle::Rect::Rect(std::string shaderid, float width, float height, glm::vec4 color)
{
	std::vector<glm::vec4> colors;
	colors.reserve(6);
	for (int i = 0; i < 6; i++) {
		colors.push_back(color);
	}
	this->_construct(shaderid, width, height, colors);
}

rgle::Rect::~Rect()
{
}

void rgle::Rect::changeDimensions(float width, float height)
{
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, -height, 0.0),
		glm::vec3(width, 0.0, 0.0),
		glm::vec3(width, -height, 0.0)
	};
	this->updateVertexBuffer();
}

void rgle::Rect::_construct(std::string& shaderid, float & width, float & height, std::vector<glm::vec4> colors)
{
	auto shader = (*this->context().manager.shader.lock())[shaderid];
	this->shader() = shader;
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(shader->programId(), "model");
	if (this->model.location < 0) {
		throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	this->color.location = glGetAttribLocation(shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, -height, 0.0),
		glm::vec3(width, 0.0, 0.0),
		glm::vec3(width, -height, 0.0)
	};

	this->index.list = {
		0,
		1,
		2,
		1,
		2,
		3
	};
	this->color.list = colors;
	this->generate();
}


rgle::Geometry3D::Geometry3D()
{
	vertex.list = {};
	index.list = {};
	color.list = {};
	uv.list = {};
	vertexArray = 0;
	samplers = {};
}

rgle::Geometry3D::Geometry3D(const Geometry3D & other)
{
	_cleanup();
	vertex = other.vertex;
	color = other.color;
	uv = other.uv;
	index = other.index;
	model = other.model;
	samplers = other.samplers;
	generate();
}

rgle::Geometry3D::Geometry3D(Geometry3D && rvalue)
{
	_cleanup();
	vertex = std::move(rvalue.vertex);
	color = std::move(rvalue.color);
	uv = std::move(rvalue.uv);
	index = std::move(rvalue.index);
	model = std::move(rvalue.model);
	vertexArray = std::move(rvalue.vertexArray);
	samplers = std::move(rvalue.samplers);
}

rgle::Geometry3D::~Geometry3D()
{
	_cleanup();
}

void rgle::Geometry3D::operator=(const Geometry3D & other)
{
	_cleanup();
	vertex = other.vertex;
	color = other.color;
	uv = other.uv;
	index = other.index;
	model = other.model;
	samplers = other.samplers;
	generate();
}

void rgle::Geometry3D::operator=(Geometry3D && rvalue)
{
	_cleanup();
	vertex = std::move(rvalue.vertex);
	color = std::move(rvalue.color);
	uv = std::move(rvalue.uv);
	index = std::move(rvalue.index);
	model = std::move(rvalue.model);
	vertexArray = std::move(rvalue.vertexArray);
	samplers = std::move(rvalue.samplers);
}

int rgle::Geometry3D::numFaces()
{
	if (!this->index.list.empty()) {
		return static_cast<int>(this->index.list.size() / 3);
	}
}

rgle::Geometry3D::Face rgle::Geometry3D::getFace(int index)
{
	if (index < 0 || index >= this->numFaces()) {
		throw Exception("failed to get face: index out of bounds", LOGGER_DETAIL_DEFAULT);
	}
	else {

		Face face = Face(
			this->vertex.list[this->index.list[index * 3]],
			this->index.list[index * 3],
			this->vertex.list[this->index.list[index * 3 + 1]],
			this->index.list[index * 3 + 1],
			this->vertex.list[this->index.list[index * 3 + 2]],
			this->index.list[index * 3 + 2]
		);
	}
}

void rgle::Geometry3D::generate()
{
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	if (!vertex.list.empty()) {
		glGenBuffers(1, &vertex.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(vertex.location);
		glVertexAttribPointer(vertex.location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	if (!color.list.empty()) {
		glGenBuffers(1, &color.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, color.buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * color.list.size() * 4, color.list.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(color.location);
		glVertexAttribPointer(color.location, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}
	if (!uv.list.empty()) {
		glGenBuffers(1, &uv.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, uv.buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uv.list.size() * 2, uv.list.data(), GL_STATIC_DRAW);
	}
	if (!index.list.empty()) {
		glGenBuffers(1, &index.buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index.buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.list.size() * sizeof(GLushort), &index.list[0], GL_STATIC_DRAW);
	}
}

void rgle::Geometry3D::standardRender(std::shared_ptr<ShaderProgram> shader)
{
	glBindVertexArray(vertexArray);
	if (model.enabled) {
		glUniformMatrix4fv(model.location, 1, GL_FALSE, &model.matrix[0][0]);
	}
	if (index.list.empty()) {
		glDrawArrays(GL_TRIANGLES, 0, vertex.list.size());
	}
	else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index.buffer);
		glDrawElements(GL_TRIANGLES, index.list.size(), GL_UNSIGNED_SHORT, nullptr);
	}
}

void rgle::Geometry3D::standardFill(Fill colorFill)
{
	bool first = true;
	glm::vec2 xrange = glm::vec2(0.0f, 0.0f);
	glm::vec2 yrange = glm::vec2(0.0f, 0.0f);
	for (int i = 0; i < this->vertex.list.size(); i++) {
		if (first || this->vertex.list[i].x < xrange.x) {
			xrange.x = this->vertex.list[i].x;
		}
		if (first || this->vertex.list[i].x > xrange.y) {
			xrange.y = this->vertex.list[i].x;
		}
		if (first || this->vertex.list[i].y < yrange.x) {
			yrange.x = this->vertex.list[i].y;
		}
		if (first || this->vertex.list[i].y > yrange.y) {
			yrange.y = this->vertex.list[i].y;
		}
	}
	this->color.list = {};
	int len = this->index.list.empty() ? this->vertex.list.size() : this->index.list.size();
	for (int i = 0; i < len; i++) {
		if (!this->index.list.empty()) {
			unsigned short index = this->index.list[i];
			if (index < this->vertex.list.size()) {
				glm::vec3& vertex = this->vertex.list[index];
				this->color.list.push_back(colorFill.evaluate((vertex.x - xrange.x) / xrange.y, (vertex.y - yrange.x) / yrange.y));
			}
			else {
				this->color.list.push_back(glm::vec4(0.0, 0.0, 0.0, 0.0));
			}
		}
		else {
			glm::vec3& vertex = this->vertex.list[i];
			this->color.list.push_back(colorFill.evaluate((vertex.x - xrange.x) / xrange.y, (vertex.y - yrange.x) / yrange.y));
		}
	}
}

void rgle::Geometry3D::updateVertexBuffer()
{
	glNamedBufferSubData(vertex.buffer, 0, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data());
}

void rgle::Geometry3D::updateIndexBuffer()
{
	glNamedBufferSubData(index.buffer, 0, sizeof(GLushort) * index.list.size(), index.list.data());
}

void rgle::Geometry3D::updateColorBuffer()
{
	glNamedBufferSubData(color.buffer, 0, sizeof(GLfloat) * color.list.size() * 4, color.list.data());
}

void rgle::Geometry3D::updateUVBuffer()
{
	glNamedBufferSubData(vertex.buffer, 0, sizeof(GLfloat) * uv.list.size() * 2, uv.list.data());
}

void rgle::Geometry3D::_cleanup()
{
	if (!vertex.list.empty()) {
		glDeleteBuffers(1, &vertex.buffer);
	}
	if (!color.list.empty()) {
		glDeleteBuffers(1, &color.buffer);
	}
	if (!uv.list.empty()) {
		glDeleteBuffers(1, &uv.buffer);
	}
	if (!index.list.empty()) {
		glDeleteBuffers(1, &index.buffer);
	}
	if (vertexArray != 0) {
		glDeleteVertexArrays(1, &vertexArray);
	}
}

rgle::ImageRect::ImageRect()
{
}

rgle::ImageRect::ImageRect(std::string shaderid, float width, float height, std::string image, ShaderModel shadermodel) :
	ImageRect(
		Sampler2D(
			ContextManager::getCurrentContext().manager.shader.lock()->getStrict(shaderid),
			std::make_shared<Texture2D>(image)
		),
		width,
		height,
		shadermodel
	)
{
}

rgle::ImageRect::ImageRect(Sampler2D sampler, float width, float height, ShaderModel shadermodel)
{
	auto shader = sampler.shader.lock();
	this->shader() = shader;
	this->samplers.push_back(sampler);
	this->model.matrix = glm::mat4(1.0f);
	if (shadermodel != ShaderModel::INSTANCED) {
		this->model.location = glGetUniformLocation(shader->programId(), "model");
		if (this->model.location < 0) {
			throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_IDENTIFIER(this->id));
		}
	}
	this->vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex position", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	this->uv.location = glGetAttribLocation(shader->programId(), "texture_coords");
	if (this->uv.location < 0) {
		throw GraphicsException("failed to locate shader attribute: texture coordinates", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	this->vertex.list = {
		glm::vec3(-width / 2, -height / 2, 0.0),
		glm::vec3(-width / 2, height / 2, 0.0),
		glm::vec3(width / 2, -height / 2, 0.0),
		glm::vec3(width / 2, height / 2, 0.0)
	};
	this->uv.list = {
		glm::vec2(0.0, 0.0),
		glm::vec2(0.0, 1.0),
		glm::vec2(1.0, 0.0),
		glm::vec2(1.0, 1.0)
	};
	this->index.list = {
		0,
		1,
		2,
		1,
		2,
		3
	};

	this->generate();
}

rgle::ImageRect::~ImageRect()
{
}

void rgle::ImageRect::render()
{
	glBindVertexArray(vertexArray);
	if (model.enabled) {
		glUniformMatrix4fv(model.location, 1, GL_FALSE, &model.matrix[0][0]);
	}

	this->samplers[0].use();

	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glEnableVertexAttribArray(vertex.location);
	glVertexAttribPointer(vertex.location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uv.buffer);
	glEnableVertexAttribArray(uv.location);
	glVertexAttribPointer(uv.location, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index.buffer);
	glDrawElements(GL_TRIANGLES, index.list.size(), GL_UNSIGNED_SHORT, nullptr);
}

rgle::SceneLayer::SceneLayer(std::string id) : RenderLayer(id)
{
}

rgle::SceneLayer::~SceneLayer()
{
}

void rgle::SceneLayer::add(std::shared_ptr<Geometry3D> geometry)
{
	this->_renderables.push_back(geometry);
}

rgle::Geometry3D::Face::Face(glm::vec3 & p1,
	unsigned short & i1,
	glm::vec3 & p2,
	unsigned short & i2,
	glm::vec3 & p3,
	unsigned short & i3) : p1(p1), i1(i1), p2(p2), i2(i2), p3(p3), i3(i3)
{
}

rgle::InstancedRenderer::InstancedRenderer(std::string id, std::shared_ptr<ViewTransformer> transformer, float allocationFactor, size_t minAllocated) :
	_allocationFactor(allocationFactor),
	_minAllocated(minAllocated),
	_transformer(transformer),
	RenderLayer(id)
{
	if (this->_allocationFactor <= 1.0f || this->_minAllocated < 1 || !std::isnormal(this->_allocationFactor)) {
		throw IllegalArgumentException("failed to create instanced renderer, invalid allocation factor", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
}

rgle::InstancedRenderer::~InstancedRenderer()
{
	for (auto it = this->_setMap.begin(); it != this->_setMap.end(); ++it) {
		glDeleteBuffers(1, &it->second.ssbo);
		delete[] it->second.instanceData;
	}
	this->_setMap.clear();
}

void rgle::InstancedRenderer::addModel(std::string key, std::shared_ptr<Geometry3D> geometry, size_t payloadsize)
{
	Logger::debug("adding model to instanced renderer with key: " + key, LOGGER_DETAIL_DEFAULT);
	if (key.empty()) {
		throw IllegalArgumentException("failed to add model to renderer, invalid key", LOGGER_DETAIL_DEFAULT);
	}
	if (this->_setMap.find(key) != this->_setMap.end()) {
		Logger::warn("instance set for model with key: " + key + " already created, all models will be destroyed", LOGGER_DETAIL_DEFAULT);
		glDeleteBuffers(1, &this->_setMap[key].ssbo);
	}
	InstanceSet set;
	set.numInstances = 0;
	set.numAllocated = this->_minAllocated;
	// NOTE: payload size must be aligned by std430 rules
	set.payloadSize = aligned_std430_size(payloadsize, 4); // TODO: replace hardcoded constant 4 (should be size of largest member)
	set.instanceData = (unsigned char*) std::calloc(this->_minAllocated, set.payloadSize);
	set.geometry = geometry;
	glBindVertexArray(geometry->vertexArray);
	glGenBuffers(1, &set.ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, set.ssbo);
	this->_setMap[key] = set;
	glBufferData(GL_SHADER_STORAGE_BUFFER, set.numAllocated * set.payloadSize, set.instanceData, GL_DYNAMIC_DRAW);
}

void rgle::InstancedRenderer::setModelBindFunc(std::string key, std::function<void()> bindfunc)
{
	auto it = this->_setMap.find(key);
	if (it == this->_setMap.end()) {
		throw NotFoundException("failed to set bind function of model: " + key, LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	else {
		it->second.bindFunc = bindfunc;
	}
}

void rgle::InstancedRenderer::setModelShader(std::string key, std::shared_ptr<ShaderProgram> shader)
{
	auto it = this->_setMap.find(key);
	if (it == this->_setMap.end()) {
		throw NotFoundException("failed to set shader of model: " + key, LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	else {
		it->second.shader = shader;
	}
}

void rgle::InstancedRenderer::removeModel(std::string key)
{
	RGLE_DEBUG_ONLY(Logger::debug("removing model from instanced renderer with key: " + key, LOGGER_DETAIL_DEFAULT);)
	if (this->_setMap.find(key) == this->_setMap.end()) {
		throw NotFoundException(
			"failed to remove model with key: " + key + " from instanced renderer, key not found",
			LOGGER_DETAIL_DEFAULT
		);
	}
	InstanceSet* set = &this->_setMap[key];
	for (auto it = set->allocationMap.begin(); it != set->allocationMap.end(); ++it) {
		this->_keyLookupTable.erase(it->first);
	}
	glDeleteBuffers(1, &set->ssbo);
	delete[] set->instanceData;
	set->instanceData = nullptr;
	this->_setMap.erase(key);
}

size_t rgle::InstancedRenderer::addInstance(std::string key, void* payload, size_t size)
{
	if (this->_setMap.find(key) == this->_setMap.end()) {
		throw NotFoundException("failed to add instance of model with key: " + key + ", key not found", LOGGER_DETAIL_DEFAULT);
	}
	size_t id = InstancedRenderer::_idCounter++;
	InstanceSet* set = &this->_setMap[key];
	if (size > set->payloadSize) {
		throw IllegalArgumentException("failed to add instance of model with key: " + key + ", invalid payload size", LOGGER_DETAIL_DEFAULT);
	}
	set->numInstances++;
	set->allocationMap[id] = set->numInstances - 1;
	this->_keyLookupTable[id] = key;
	if (set->numInstances > set->numAllocated) {
		set->numAllocated *= this->_allocationFactor;
		set->instanceData = (unsigned char*) std::realloc(
			set->instanceData,
			set->numAllocated * set->payloadSize
		);
		std::memcpy(set->instanceData + (set->numInstances - 1) * set->payloadSize, payload, size);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_setMap[key].ssbo);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			set->payloadSize * set->numAllocated,
			set->instanceData,
			GL_DYNAMIC_DRAW
		);
	}
	else {
		void* dst = set->instanceData + (set->numInstances - 1) * set->payloadSize;
		std::memcpy(dst, payload, size);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, set->ssbo);
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER,
			(set->numInstances - 1) * set->payloadSize,
			set->payloadSize,
			dst
		);
	}
	return id;
}

size_t rgle::InstancedRenderer::addInstance(std::string key, glm::mat4 model)
{
	return this->addInstance(key, &model[0][0], 16 * sizeof(GLfloat));
}

void rgle::InstancedRenderer::updateInstance(size_t id, void * payload, size_t offset, size_t size)
{
	if (this->_keyLookupTable.find(id) == this->_keyLookupTable.end()) {
		throw NotFoundException(
			"failed to update instance: " + std::to_string(id) + ", failed to lookup key",
			LOGGER_DETAIL_IDENTIFIER(this->id)
		);
	}
	std::string& key = this->_keyLookupTable[id];
	if (this->_setMap.find(key) == this->_setMap.end()) {
		throw NotFoundException(
			"failed to update instance: " + std::to_string(id) + ", instance set with key: " + key + " not found",
			LOGGER_DETAIL_IDENTIFIER(this->id)
		);
	}
	InstanceSet* set = &this->_setMap[key];
	if (size > set->payloadSize || size == 0) {
		throw IllegalArgumentException(
			"invalid payload size while updating instance: " + std::to_string(id),
			LOGGER_DETAIL_IDENTIFIER(this->id)
		);
	}
	if (offset >= set->payloadSize || offset + size > set->payloadSize) {
		throw OutOfBoundsException(LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	size_t idx = set->allocationMap[id];
	void* dst = set->instanceData + idx * set->payloadSize + offset;
	std::memcpy(dst, payload, size);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, set->ssbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx * set->payloadSize + offset, size, dst);
}

void rgle::InstancedRenderer::removeInstance(size_t id)
{
	std::string key = this->_keyLookupTable[id];
	if (this->_setMap.find(key) == this->_setMap.end()) {
		throw NotFoundException("failed to remove a model instance with id: " + std::to_string(id) + ", id not found", LOGGER_DETAIL_DEFAULT);
	}
	InstanceSet* set = &this->_setMap[key];
	size_t idx = set->allocationMap[id];
	set->numInstances--;
	for (auto it = set->allocationMap.begin(); it != set->allocationMap.end(); ++it) {
		if (it->second == set->numInstances) {
			it->second = idx;
		}
	}
	set->allocationMap.erase(id);
	this->_keyLookupTable.erase(id);
	void* idxptr = set->instanceData + idx * set->payloadSize;
	// Copy last instance payload into deleted instance payload slot
	std::memcpy(idxptr, set->instanceData + set->numInstances * set->payloadSize, set->payloadSize);
	size_t reduced = set->numAllocated / this->_allocationFactor;
	if (set->numInstances <= reduced && reduced >= this->_minAllocated) {
		set->numAllocated = reduced;
		set->instanceData = (unsigned char*)std::realloc(set->instanceData, set->numAllocated * set->payloadSize);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, set->ssbo);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			set->numInstances * set->payloadSize,
			set->instanceData,
			GL_DYNAMIC_DRAW
		);
	}
	else {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, set->ssbo);
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER,
			idx * set->payloadSize,
			set->payloadSize,
			idxptr
		);
	}
}

void rgle::InstancedRenderer::render()
{
	for (auto it = this->_setMap.begin(); it != this->_setMap.end(); ++it) {
		if (!it->second.shader.expired()) {
			it->second.shader.lock()->use();
			this->_transformer->bind(it->second.shader.lock());
		}
		else {
			auto shader = this->shaderLocked();
			shader->use();
			this->_transformer->bind(shader);
		}
		glBindVertexArray(it->second.geometry->vertexArray);

		if (it->second.bindFunc) {
			it->second.bindFunc();
		}

		for (int i = 0; i < it->second.geometry->samplers.size(); i++) {
			it->second.geometry->samplers[i].use();
		}

		if (!it->second.geometry->vertex.list.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, it->second.geometry->vertex.buffer);
			glEnableVertexAttribArray(it->second.geometry->vertex.location);
			glVertexAttribPointer(it->second.geometry->vertex.location, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (!it->second.geometry->uv.list.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, it->second.geometry->uv.buffer);
			glEnableVertexAttribArray(it->second.geometry->uv.location);
			glVertexAttribPointer(it->second.geometry->uv.location, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, it->second.ssbo);
		if (it->second.geometry->index.list.empty()) {
			glDrawArraysInstanced(GL_TRIANGLES, 0, it->second.geometry->vertex.list.size(), it->second.numInstances);
		}
		else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->second.geometry->index.buffer);
			glDrawElementsInstanced(
				GL_TRIANGLES,
				it->second.geometry->index.list.size(),
				GL_UNSIGNED_SHORT,
				nullptr,
				it->second.numInstances
			);
		}
	}
}

void rgle::InstancedRenderer::update()
{
}

const char * rgle::InstancedRenderer::typeName() const
{
	return "rgle::InstancedRenderer";
}

size_t rgle::aligned_std140_size(const size_t & size, const size_t & largestMember)
{
	// Round largestMember up by sizeof(vec4)
	size_t std140BaseAlignment = (4 * sizeof(GLfloat)) * std::ceilf((float)largestMember / (4 * sizeof(GLfloat)));
	// Round size up by std140BaseAlignment
	return std140BaseAlignment * std::ceilf((float)size / std140BaseAlignment);
}

size_t rgle::aligned_std430_size(const size_t & size, const size_t & largestMember)
{
	// Round size up by largestMember
	return largestMember * std::ceilf((float)size / largestMember);
}
