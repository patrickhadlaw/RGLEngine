#include "rgle/gfx/CharRect.h"

rgle::gfx::CharRect::CharRect()
{
}

rgle::gfx::CharRect::CharRect(std::shared_ptr<ShaderProgram> shader, std::shared_ptr<res::Glyph> glyph, UnitVector2D offset, float zIndex, UnitValue baselineOffset, UnitVector2D dimensions)
{
	auto window = this->context().window.lock();
	this->shader() = shader;
	this->offset = offset;
	this->zIndex = zIndex;
	this->baselineOffset = baselineOffset;
	this->dimensions = dimensions;
	this->color.location = glGetAttribLocation(shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}
	this->uv.location = glGetAttribLocation(shader->programId(), "texture_coords");
	if (this->uv.location < 0) {
		throw Exception("failed to locate shader attribute: texture coordinates", LOGGER_DETAIL_DEFAULT);
	}
	this->model.enabled = false;
	float resolvedWidth = this->dimensions.x.resolve(window, Window::X);
	float resolvedHeight = this->dimensions.y.resolve(window, Window::Y);
	glm::vec3 scaled = glm::vec3(
		offset.x.resolve(window),
		-offset.y.resolve(window, Window::Y) + baselineOffset.resolve(window, Window::Y),
		zIndex
	);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0) + scaled,
		glm::vec3(resolvedWidth, 0.0, 0.0) + scaled,
		glm::vec3(0.0, resolvedHeight, 0.0) + scaled,
		glm::vec3(resolvedWidth, resolvedHeight, 0.0) + scaled
	};
	float uvWidth = (float)glyph->border.width / glyph->image->width;
	float uvHeight = (float)glyph->border.height / glyph->image->height;
	this->uv.list = {
		glm::vec2(0.0, uvHeight),
		glm::vec2(uvWidth, uvHeight),
		glm::vec2(0.0, 0.0),
		glm::vec2(uvWidth, 0.0)
	};
	this->index.list = {
		0,
		1,
		2,
		1,
		2,
		3
	};
	this->glyph = glyph;
	this->samplers = { Sampler2D(shader, glyph->texture) };
	this->generate();
}

rgle::gfx::CharRect::CharRect(const CharRect & other)
{
	_cleanup();
	width = other.width;
	height = other.height;
	zIndex = other.zIndex;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	dimensions = other.dimensions;
	glyph = other.glyph;
	color = other.color;
	samplers = other.samplers;
	vertex = other.vertex;
	index = other.index;
	uv = other.uv;
	this->generate();
}

rgle::gfx::CharRect::CharRect(CharRect && rvalue)
{
	_cleanup();
	width = rvalue.width;
	height = rvalue.height;
	zIndex = rvalue.zIndex;
	offset = std::move(rvalue.offset);
	baselineOffset = rvalue.baselineOffset;
	dimensions = rvalue.dimensions;
	glyph = rvalue.glyph;
	color = std::move(rvalue.color);
	samplers = rvalue.samplers;
	vertex = std::move(rvalue.vertex);
	index = std::move(rvalue.index);
	uv = std::move(rvalue.uv);
	model = std::move(rvalue.model);
	vertexArray = rvalue.vertexArray;
	rvalue.vertexArray = 0;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

rgle::gfx::CharRect::~CharRect()
{
}

void rgle::gfx::CharRect::operator=(const CharRect & other)
{
	_cleanup();
	width = other.width;
	height = other.height;
	zIndex = other.zIndex;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	dimensions = other.dimensions;
	glyph = other.glyph;
	color = other.color;
	samplers = other.samplers;
	vertexArray = other.vertexArray;
	vertex = other.vertex;
	index = other.index;
	uv = other.uv;
	this->generate();
}

void rgle::gfx::CharRect::operator=(CharRect && rvalue)
{
	this->_cleanup();
	width = rvalue.width;
	height = rvalue.height;
	zIndex = rvalue.zIndex;
	offset = std::move(rvalue.offset);
	baselineOffset = rvalue.baselineOffset;
	dimensions = rvalue.dimensions;
	glyph = rvalue.glyph;
	color = std::move(rvalue.color);
	samplers = rvalue.samplers;
	vertex = std::move(rvalue.vertex);
	index = std::move(rvalue.index);
	uv = std::move(rvalue.uv);
	model = std::move(rvalue.model);
	vertexArray = rvalue.vertexArray;
	rvalue.vertexArray = 0;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

void rgle::gfx::CharRect::recalculate()
{
	auto window = this->context().window.lock();
	float resolvedWidth = this->dimensions.x.resolve(window, Window::X);
	float resolvedHeight = this->dimensions.y.resolve(window, Window::Y);
	glm::vec3 scaled = glm::vec3(
		offset.x.resolve(window),
		-offset.y.resolve(window, Window::Y) + baselineOffset.resolve(window, Window::Y),
		zIndex
	);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0) + scaled,
		glm::vec3(resolvedWidth, 0.0, 0.0) + scaled,
		glm::vec3(0.0, resolvedHeight, 0.0) + scaled,
		glm::vec3(resolvedWidth, resolvedHeight, 0.0) + scaled
	};
	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data());
}

void rgle::gfx::CharRect::render()
{
	glBindVertexArray(vertexArray);

	this->samplers[0].use();

	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glEnableVertexAttribArray(vertex.location);
	glVertexAttribPointer(vertex.location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uv.buffer);
	glEnableVertexAttribArray(uv.location);
	glVertexAttribPointer(uv.location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index.buffer);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index.list.size()), GL_UNSIGNED_SHORT, nullptr);
}
