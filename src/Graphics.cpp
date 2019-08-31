#include "Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

rgle::GraphicsException::GraphicsException() : Exception()
{
}

rgle::GraphicsException::GraphicsException(std::string except, Logger::Detail & detail) : Exception(except, detail)
{
}

rgle::GraphicsException::~GraphicsException()
{
}

std::string rgle::GraphicsException::_type()
{
	return std::string("rgle::GraphicsException");
}

rgle::Shape::Shape()
{
	this->model.matrix = glm::mat4(1.0f);
}

rgle::Shape::Shape(Context context, std::string shader, std::vector<glm::vec3> verticies, std::vector<glm::vec4> colors)
{
	this->_context = context;
	this->color.list = colors;
	this->vertex.list = verticies;
	this->shader = (*_context.manager.shader)[shader];

	model.matrix = glm::mat4(1.0f);
	model.location = glGetUniformLocation(this->shader->programId(), "model");
	if (model.location < 0) {
		throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	color.location = glGetAttribLocation(this->shader->programId(), "vertex_color");
	if (color.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	vertex.location = glGetAttribLocation(this->shader->programId(), "vertex_position");
	if (vertex.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}

	this->generate();
}

rgle::Shape::Shape(Context context, std::string shader, std::vector<glm::vec3> verticies, glm::vec4 color)
{
	std::vector<glm::vec4> colors;
	colors.resize(verticies.size());
	for (int i = 0; i < colors.size(); i++) {
		colors[i] = color;
	}
	Shape(context, shader, verticies, colors);
}

rgle::Shape::~Shape()
{

}

void rgle::Shape::render()
{
	this->standardRender(this->shader);
}

std::string & rgle::Shape::typeName()
{
	return std::string("rgle::Shape");
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

rgle::Image::Image()
{
	image = nullptr;
	width = -1;
	height = -1;
	bpp = -1;
}

rgle::Image::Image(std::string imagefile)
{
	RGLE_DEBUG_ONLY(rgle::Logger::debug("loading image: " + imagefile, LOGGER_DETAIL_DEFAULT);)
	image = stbi_load(imagefile.data(), &width, &height, &bpp, STBI_rgb_alpha);
	if (this->image == nullptr) {
		throw IOException("failed to load image: " + imagefile, LOGGER_DETAIL_DEFAULT);
	}
}

rgle::Image::Image(const Image & other)
{
	if (other.image == nullptr) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	this->width = other.width;
	this->height = other.height;
	this->bpp = other.bpp;
	this->image = new unsigned char[width * bpp * height];
	for (int i = 0; i < width * bpp * height; i++) {
		this->image[i] = other.image[i];
	}
}

rgle::Image::Image(Image && rvalue)
{
	this->width = rvalue.width;
	this->height = rvalue.height;
	this->bpp = rvalue.bpp;
	this->image = rvalue.image;
	rvalue.image = nullptr;
}

void rgle::Image::operator=(const Image& other)
{
	delete[] image;
	image = nullptr;
	this->width = other.width;
	this->height = other.height;
	this->bpp = other.bpp;
	this->image = other.image;
	this->image = new unsigned char[width * bpp * height];
	for (int i = 0; i < width * bpp * height; i++) {
		this->image[i] = other.image[i];
	}
}

rgle::Image::~Image()
{
	delete[] image;
	image = nullptr;
}

rgle::Texture::Texture()
{
	this->texture = GL_TEXTURE0;
	this->format.internal = GL_RGBA8;
	this->format.target = GL_RGBA;
	this->image = nullptr;
	this->textureID = 0;
}

rgle::Texture::Texture(std::string imagefile, GLenum texture, Format format)
{
	this->format = format;
	this->image = std::make_shared<Image>(Image(imagefile));
	this->texture = texture;
	this->_generate();
}

rgle::Texture::Texture(const Texture & other)
{
	this->image = other.image;
	this->format = other.format;
	this->texture = other.texture;
	this->_generate();
}

rgle::Texture::Texture(Texture && rvalue)
{
	this->image = rvalue.image;
	this->format = rvalue.format;
	this->texture = rvalue.texture;
	this->textureID = rvalue.textureID;
	rvalue.textureID = 0;
}

rgle::Texture::Texture(const std::shared_ptr<Image> & image, GLenum texture, Format format)
{
	this->format = format;
	this->image = image;
	this->texture = texture;
	this->_generate();
}

rgle::Texture::~Texture()
{
	glDeleteTextures(1, &textureID);
	textureID = 0;
}

void rgle::Texture::operator=(const Texture & other)
{
	glDeleteTextures(1, &textureID);
	this->format = other.format;
	this->texture = other.texture;
	this->image = std::make_shared<Image>(Image(*other.image));
	this->_generate();
}

void rgle::Texture::operator=(Texture && rvalue)
{
	glDeleteTextures(1, &textureID);
	this->image = rvalue.image;
	this->format = rvalue.format;
	this->texture = rvalue.texture;
	this->textureID = rvalue.textureID;
	rvalue.textureID = 0;
}

void rgle::Texture::_generate()
{
	if (this->image == nullptr) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		format.internal,
		this->image->width,
		this->image->height,
		0,
		format.target,
		GL_UNSIGNED_BYTE,
		this->image->image
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

rgle::Sampler2D::Sampler2D()
{
	this->enabled = true;
	this->shader = nullptr;
	this->texture = nullptr;
	this->samplerLocation = 0;
}

rgle::Sampler2D::Sampler2D(std::shared_ptr<ShaderProgram> shader, std::string imagefile, GLenum texture, Texture::Format format)
{
	this->enabled = true;
	this->shader = shader;
	this->texture = std::make_shared<Texture>(Texture(std::make_shared<Image>(Image(imagefile)), texture, format));
	this->_generate();
}

rgle::Sampler2D::Sampler2D(const Sampler2D & other)
{
	this->enabled = other.enabled;
	this->shader = other.shader;
	this->texture = other.texture;
	this->_generate();
}

rgle::Sampler2D::Sampler2D(std::shared_ptr<ShaderProgram> shader, const std::shared_ptr<Texture>& texture)
{
	this->enabled = true;
	this->shader = shader;
	this->texture = texture;
	this->_generate();
}

void rgle::Sampler2D::operator=(const Sampler2D& other)
{
	this->enabled = other.enabled;
	this->shader = other.shader;
	this->texture = other.texture;
	this->_generate();
}

rgle::Sampler2D::~Sampler2D()
{
}

void rgle::Sampler2D::use()
{
	if (this->texture == nullptr || this->shader == nullptr) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	glUniform1i(this->enableLocation, enabled);
	glUniform1i(this->samplerLocation, 0);
	glActiveTexture(texture->texture);
	glBindTexture(GL_TEXTURE_2D, texture->textureID);
}

void rgle::Sampler2D::_generate()
{
	this->enableLocation = glGetUniformLocation(this->shader->programId(), "enable_texture");
	if (this->enableLocation < 0) {
		throw Exception("failed to get uniform location of: enable texture", LOGGER_DETAIL_DEFAULT);
	}
	this->samplerLocation = glGetUniformLocation(this->shader->programId(), "texture_0");
	if (this->samplerLocation < 0) {
		throw Exception("failed to get uniform location of: texture 0", LOGGER_DETAIL_DEFAULT);
	}
}

rgle::Triangle::Triangle()
{
}

rgle::Triangle::Triangle(Context context, std::string shader, float a, float b, float theta, glm::vec4 color)
{	
	this->_construct(context, shader, a, b, theta, std::vector<glm::vec4>({ color, color, color }));
}

rgle::Triangle::Triangle(Context context, std::string shader, float a, float b, float theta, std::vector<glm::vec4> colors)
{
	this->_construct(context, shader, a, b, theta, colors);
}

void rgle::Triangle::_construct(Context& context, std::string& shader, float& a, float& b, float& theta, std::vector<glm::vec4> colors)
{
	this->_context = context;
	this->shader = (*context.manager.shader)[shader];
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(this->shader->programId(), "model");
	if (this->model.location < 0) {
		throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	this->color.location = glGetAttribLocation(this->shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(this->shader->programId(), "vertex_position");
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

rgle::Rect::Rect(Context context, std::string shader, float width, float height, std::vector<glm::vec4> colors)
{
	this->_construct(context, shader, width, height, colors);
}

rgle::Rect::Rect(Context context, std::string shader, float width, float height, glm::vec4 color)
{
	std::vector<glm::vec4> colors;
	colors.reserve(6);
	for (int i = 0; i < 6; i++) {
		colors.push_back(color);
	}
	this->_construct(context, shader, width, height, colors);
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

void rgle::Rect::_construct(Context& context, std::string& shader, float & width, float & height, std::vector<glm::vec4> colors)
{
	this->shader = (*context.manager.shader)[shader];
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(this->shader->programId(), "model");
	if (this->model.location < 0) {
		throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	this->color.location = glGetAttribLocation(this->shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(this->shader->programId(), "vertex_position");
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
	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data());
}

void rgle::Geometry3D::updateIndexBuffer()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index.buffer);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLushort) * index.list.size(), index.list.data());
}

void rgle::Geometry3D::updateColorBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, color.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * color.list.size() * 4, color.list.data());
}

void rgle::Geometry3D::updateUVBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * uv.list.size() * 2, uv.list.data());
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

rgle::ImageRect::ImageRect(Context context, std::string shader, float width, float height, std::string image)
{
	this->_context = context;
	this->shader = (*context.manager.shader)[shader];
	this->samplers.push_back(Sampler2D(this->shader, image));
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(this->shader->programId(), "model");
	if (this->model.location < 0) {
		throw GraphicsException("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(this->shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw GraphicsException("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}
	this->uv.location = glGetAttribLocation(this->shader->programId(), "texture_coords");
	if (this->uv.location < 0) {
		throw GraphicsException("failed to locate shader attribute: texture coordinates", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.list = {
		glm::vec3(-width / 2, height / 2, 0.0),
		glm::vec3(-width / 2, -height / 2, 0.0),
		glm::vec3(width / 2, height / 2, 0.0),
		glm::vec3(width / 2, -height / 2, 0.0)
	};
	this->uv.list = {
		glm::vec2(0.0, 1.0),
		glm::vec2(0.0, 0.0),
		glm::vec2(1.0, 1.0),
		glm::vec2(1.0, 0.0)
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
