#include "Graphics.h"

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
}

cppogl::Image::Image(std::string imagefile)
{
	image = cimg_library::CImg<unsigned char>(imagefile.data());
}

cppogl::Image::~Image()
{
}

cppogl::Sampler2D::Sampler2D()
{
}

cppogl::Sampler2D::Sampler2D(std::string uniformname, std::string imagefile) : Image(imagefile)
{
	glGenTextures(1, &textureID);
}

cppogl::Sampler2D::~Sampler2D()
{
}

void cppogl::Sampler2D::use()
{
	glBindTexture(GL_TEXTURE_2D, textureID);

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

cppogl::Rect::Rect(sShaderProgram shader, float width, float height, glm::vec4 color)
{
	this->shader = shader;
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(shader->id(), "model");
	if (this->model.location < 0) {
		throw std::runtime_error("failed to locate shader uniform");
	}
	this->color.location = glGetAttribLocation(shader->id(), "vertex_color");
	if (this->color.location < 0) {
		throw std::runtime_error("failed to locate shader attribute");
	}
	this->vertex.location = glGetAttribLocation(shader->id(), "vertex_position");
	if (this->vertex.location < 0) {
		throw std::runtime_error("failed to locate shader attribute");
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
	this->color.list.reserve(this->vertex.list.size());
	for (int i = 0; i < this->color.list.size(); i++) {
		this->color.list[i] = color;
	}
	this->generate();
}

cppogl::Rect::~Rect()
{
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
