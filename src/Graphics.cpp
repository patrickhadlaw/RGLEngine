#include "Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

cppogl::Shape::Shape()
{
	this->model.matrix = glm::mat4(1.0f);
}

cppogl::Shape::Shape(sShaderProgram shader, std::vector<glm::vec3> verticies, std::vector<glm::vec4> colors)
{
	this->color.list = colors;
	this->vertex.list = verticies;

	model.matrix = glm::mat4(1.0f);
	model.location = glGetUniformLocation(shader->id(), "model");
	if (model.location < 0) {
		throw std::runtime_error("failed to locate shader uniform: model matrix");
	}
	color.location = glGetAttribLocation(shader->id(), "vertex_color");
	if (color.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: vertex color");
	}
	vertex.location = glGetAttribLocation(shader->id(), "vertex_position");
	if (vertex.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: vertex position");
	}

	this->generate();
}

cppogl::Shape::Shape(sShaderProgram shader, std::vector<glm::vec3> verticies, glm::vec4 color)
{
	std::vector<glm::vec4> colors;
	colors.resize(verticies.size());
	for (int i = 0; i < colors.size(); i++) {
		colors[i] = color;
	}
	Shape(shader, verticies, colors);
}

cppogl::Shape::~Shape()
{
	glDeleteBuffers(1, &vertex.buffer);
	glDeleteBuffers(1, &color.buffer);
	glDeleteVertexArrays(1, &vertexArray);
}

void cppogl::Shape::render()
{
	glBindVertexArray(vertexArray);
	glUniformMatrix4fv(model.location, 1, GL_FALSE, &model.matrix[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glEnableVertexAttribArray(vertex.location);
	glVertexAttribPointer(vertex.location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, color.buffer);
	glEnableVertexAttribArray(color.location);
	glVertexAttribPointer(color.location, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, vertex.list.size());
}

const std::string & cppogl::Shape::typeName()
{
	return "cppogl::Shape";
}

void cppogl::Shape::translate(float x, float y, float z)
{
	glm::mat4 translate(1.0f);
	translate[3][0] = x;
	translate[3][1] = y;
	translate[3][2] = z;
	model.matrix = model.matrix*translate;
}

void cppogl::Shape::rotate(float x, float y, float z)
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

cppogl::Image::Image()
{
	image = nullptr;
	width = -1;
	height = -1;
	bpp = -1;
}

cppogl::Image::Image(std::string imagefile)
{
	image = stbi_load(imagefile.data(), &width, &height, &bpp, STBI_rgb_alpha);
	if (this->image == nullptr) {
		throw std::runtime_error("failed to load image");
	}
}

cppogl::Image::Image(Image && rvalue)
{
	this->width = rvalue.width;
	this->height = rvalue.height;
	this->bpp = rvalue.bpp;
	this->image = rvalue.image;
	rvalue.image = nullptr;
}

void cppogl::Image::operator=(const Image& other)
{
	this->width = other.width;
	this->height = other.height;
	this->bpp = other.bpp;
	this->image = other.image;
	this->image = new unsigned char[width * bpp * height];
	for (int i = 0; i < width * bpp * height; i++) {
		this->image[i] = other.image[i];
	}
}

cppogl::Image::~Image()
{
	delete[] image;
	image = nullptr;
}

cppogl::Sampler2D::Sampler2D()
{
	this->texture = GL_TEXTURE0;
}

cppogl::Sampler2D::Sampler2D(sShaderProgram shader, std::string imagefile, GLenum texture) : Image(imagefile)
{
	this->texture = texture;
	this->shader = shader;
	this->_generate();
}

cppogl::Sampler2D::Sampler2D(const Sampler2D & other)
{
	this->width = other.width;
	this->height = other.height;
	this->bpp = other.bpp;
	this->image = other.image;
	this->texture = other.texture;
	this->textureID = other.textureID;
	this->image = new unsigned char[width * bpp * height];
	for (int i = 0; i < width * bpp * height; i++) {
		this->image[i] = other.image[i];
	}
	this->_generate();
}

cppogl::Sampler2D::Sampler2D(Sampler2D && rvalue)
{
	this->width = rvalue.width;
	this->height = rvalue.height;
	this->bpp = rvalue.bpp;
	this->image = rvalue.image;
	this->shader = rvalue.shader;
	this->texture = rvalue.texture;
	rvalue.image = nullptr;
	this->_generate();
}

void cppogl::Sampler2D::operator=(const Sampler2D& other)
{
	this->shader = other.shader;
	this->width = other.width;
	this->height = other.height;
	this->bpp = other.bpp;
	this->image = new unsigned char[width * bpp * height];
	for (int i = 0; i < width * bpp * height; i++) {
		this->image[i] = other.image[i];
	}
	this->_generate();
}

cppogl::Sampler2D::~Sampler2D()
{
	glDeleteTextures(1, &textureID);
}

void cppogl::Sampler2D::use()
{
	glUniform1i(this->enableLocation, 1);
	glUniform1i(this->samplerLocation, 0);
	glActiveTexture(texture);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void cppogl::Sampler2D::disable()
{
	glUniform1i(this->enableLocation, 0);
}

void cppogl::Sampler2D::_generate()
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	checkGLErrors(__LINE__);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	this->enableLocation = glGetUniformLocation(this->shader->id(), "enable_texture");
	if (this->enableLocation < 0) {
		throw std::runtime_error("failed to get uniform location of: enable texture");
	}
	this->samplerLocation = glGetUniformLocation(this->shader->id(), "texture_0");
	if (this->samplerLocation < 0) {
		throw std::runtime_error("failed to get uniform location of: texture 0");
	}
}

cppogl::Triangle::Triangle()
{
}

cppogl::Triangle::Triangle(sShaderProgram shader, float a, float b, float theta, glm::vec4 color)
{	
	this->_construct(shader, a, b, theta, std::vector<glm::vec4>({ color, color, color }));
}

cppogl::Triangle::Triangle(sShaderProgram shader, float a, float b, float theta, std::vector<glm::vec4> colors)
{
	this->_construct(shader, a, b, theta, colors);
}

void cppogl::Triangle::_construct(sShaderProgram& shader, float& a, float& b, float& theta, std::vector<glm::vec4> colors)
{
	this->shader = shader;
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(shader->id(), "model");
	if (this->model.location < 0) {
		throw std::runtime_error("failed to locate shader uniform: model matrix");
	}
	this->color.location = glGetAttribLocation(shader->id(), "vertex_color");
	if (this->color.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: vertex color");
	}
	this->vertex.location = glGetAttribLocation(shader->id(), "vertex_position");
	if (this->vertex.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: vertex position");
	}
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(a, 0.0, 0.0),
		glm::vec3(b * cosf(theta), b * sinf(theta), 0.0)
	};
	this->shader = shader;
	if (colors.size() < 3) {
		throw std::out_of_range("invalid vector size in Triangle::Triangle, expected size: 3");
	}
	this->color.list = colors;
	this->generate();
}

cppogl::Triangle::~Triangle()
{
}

cppogl::Rect::Rect()
{
}

cppogl::Rect::Rect(sShaderProgram shader, float width, float height, std::vector<glm::vec4> colors)
{
	this->_construct(shader, width, height, colors);
}

cppogl::Rect::Rect(sShaderProgram shader, float width, float height, glm::vec4 color)
{
	std::vector<glm::vec4> colors;
	colors.reserve(6);
	for (int i = 0; i < 6; i++) {
		colors.push_back(color);
	}
	this->_construct(shader, width, height, colors);
}

cppogl::Rect::~Rect()
{
}

void cppogl::Rect::_construct(sShaderProgram & shader, float & width, float & height, std::vector<glm::vec4> colors)
{
	this->shader = shader;
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(shader->id(), "model");
	if (this->model.location < 0) {
		throw std::runtime_error("failed to locate shader uniform: model matrix");
	}
	this->color.location = glGetAttribLocation(shader->id(), "vertex_color");
	if (this->color.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: vertex color");
	}
	this->vertex.location = glGetAttribLocation(shader->id(), "vertex_position");
	if (this->vertex.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: vertex position");
	}
	this->vertex.list = {
		glm::vec3(-width / 2, height / 2, 0.0),
		glm::vec3(-width / 2, -height / 2, 0.0),
		glm::vec3(width / 2, height / 2, 0.0),
		glm::vec3(-width / 2, -height / 2, 0.0),
		glm::vec3(width / 2, height / 2, 0.0),
		glm::vec3(width / 2, -height / 2, 0.0)
	};
	this->shader = shader;
	this->color.list = colors;
	this->generate();
}


cppogl::Geometry3D::Geometry3D()
{
	shader = nullptr;
	vertex.list = {};
	indicies = {};
	color.list = {};
	uv.list = {};
	samplers = {};
}

cppogl::Geometry3D::~Geometry3D()
{
}

void cppogl::Geometry3D::update()
{
}

void cppogl::Geometry3D::render()
{
}

void cppogl::Geometry3D::generate()
{
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	if (!vertex.list.empty()) {
		glGenBuffers(1, &vertex.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data(), GL_STATIC_DRAW);
	}
	if (!color.list.empty()) {
		glGenBuffers(1, &color.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, color.buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * color.list.size() * 4, color.list.data(), GL_STATIC_DRAW);
	}
	if (!uv.list.empty()) {
		glGenBuffers(1, &uv.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, uv.buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uv.list.size() * 2, uv.list.data(), GL_STATIC_DRAW);
	}
	
}

cppogl::ImageRect::ImageRect()
{
}

cppogl::ImageRect::ImageRect(sShaderProgram shader, float width, float height, std::string image)
{
	this->shader = shader;
	this->samplers.push_back(Sampler2D(shader, image));
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(shader->id(), "model");
	if (this->model.location < 0) {
		throw std::runtime_error("failed to locate shader uniform: model matrix");
	}
	this->vertex.location = glGetAttribLocation(shader->id(), "vertex_position");
	if (this->vertex.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: vertex position");
	}
	this->uv.location = glGetAttribLocation(shader->id(), "texture_coords");
	if (this->uv.location < 0) {
		throw std::runtime_error("failed to locate shader attribute: texture coordinates");
	}
	this->vertex.list = {
		glm::vec3(-width / 2, height / 2, 0.0),
		glm::vec3(-width / 2, -height / 2, 0.0),
		glm::vec3(width / 2, height / 2, 0.0),
		glm::vec3(-width / 2, -height / 2, 0.0),
		glm::vec3(width / 2, height / 2, 0.0),
		glm::vec3(width / 2, -height / 2, 0.0)
	};
	this->uv.list = {
		glm::vec2(0.0, 1.0),
		glm::vec2(0.0, 0.0),
		glm::vec2(1.0, 1.0),
		glm::vec2(0.0, 0.0),
		glm::vec2(1.0, 1.0),
		glm::vec2(1.0, 0.0)
	};
	this->shader = shader;
	this->generate();
}

cppogl::ImageRect::~ImageRect()
{
}

void cppogl::ImageRect::render()
{
	glBindVertexArray(vertexArray);
	glUniformMatrix4fv(model.location, 1, GL_FALSE, &model.matrix[0][0]);

	this->samplers[0].use();
	checkGLErrors(__LINE__);

	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glEnableVertexAttribArray(vertex.location);
	glVertexAttribPointer(vertex.location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uv.buffer);
	glEnableVertexAttribArray(uv.location);
	glVertexAttribPointer(uv.location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	checkGLErrors(__LINE__);

	glDrawArrays(GL_TRIANGLES, 0, vertex.list.size());
	checkGLErrors(__LINE__);
}
