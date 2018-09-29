#include "Font.h"
#include "Graphics.h"



cppogl::Font::Font()
{
	this->_window = nullptr;
}

cppogl::Font::Font(sWindow window, std::string fontfile)
{
	this->_window = window;
	int error = FT_Init_FreeType(&_library);
	if (error) {
		throw Exception("failed to initialise FreeType library module, error code: " + std::to_string(error), EXCEPT_DETAIL_DEFAULT);
	}
	error = FT_New_Face(_library, fontfile.data(), 0, &_face);
	if (error == FT_Err_Unknown_File_Format) {
		throw Exception("failed to load font, invalid file format", EXCEPT_DETAIL_DEFAULT);
	}
	else if (error) {
		throw Exception("failed to load font: " + fontfile, EXCEPT_DETAIL_DEFAULT);
	}
}

cppogl::Font::~Font()
{
}

void cppogl::Font::generate(int size)
{
	if (this->_window == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	FT_Set_Pixel_Sizes(_face, 0, size);
	std::map<char, sGlyph> glyphs;
	for (unsigned char c = 1; c < 128; c++) {
		if (FT_Load_Glyph(_face, FT_Get_Char_Index(_face, c), FT_LOAD_DEFAULT)) {
			throw Exception("failed to load glyph", EXCEPT_DETAIL_DEFAULT);
		}
		FT_Glyph ft_glyph;
		if (FT_Get_Glyph(_face->glyph, &ft_glyph)) {
			throw Exception("failed to get glyph", EXCEPT_DETAIL_DEFAULT);
		}
		glyphs[c] = std::make_shared<Glyph>(Glyph(ft_glyph));
	}
	this->_generated[size] = glyphs;
}

cppogl::CharRect::CharRect()
{
	this->shader = nullptr;
	this->window = nullptr;
}

cppogl::CharRect::CharRect(sWindow window, sShaderProgram shader, sGlyph glyph, glm::vec3 offset)
{
	this->shader = shader;
	this->window = window;
	this->offset = offset;
	this->color.location = glGetAttribLocation(shader->id(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", EXCEPT_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->id(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", EXCEPT_DETAIL_DEFAULT);
	}
	this->uv.location = glGetAttribLocation(shader->id(), "texture_coords");
	if (this->uv.location < 0) {
		throw Exception("failed to locate shader attribute: texture coordinates", EXCEPT_DETAIL_DEFAULT);
	}
	float width = window->parseUnit((float)glyph->border.width, Window::Unit::PT) / window->width();
	float height = window->parseUnit((float)glyph->border.height, Window::Unit::PT) / window->height();
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0) + offset,
		glm::vec3(width, 0.0, 0.0) + offset,
		glm::vec3(0.0, height, 0.0) + offset,
		glm::vec3(width, height, 0.0) + offset
	};
	this->uv.list = {
		glm::vec2(0.0, 1.0),
		glm::vec2(1.0, 1.0),
		glm::vec2(0.0, 0.0),
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
	this->glyph = glyph;
	this->samplers = { Sampler2D(shader, glyph->image, GL_TEXTURE0, Sampler2D::Format{ GL_ALPHA8, GL_ALPHA8 }) };
	this->generate();
}

cppogl::CharRect::CharRect(const CharRect & other)
{
	width = other.width;
	height = other.height;
	offset = other.offset;
	shader = other.shader;
	window = other.window;
	glyph = other.glyph;
	color = other.color;
	samplers = other.samplers;
	vertex = other.vertex;
	index = other.index;
	uv = other.uv;
	vertexArray = other.vertexArray;
	this->generate();
}

cppogl::CharRect::CharRect(CharRect && rvalue)
{
	width = rvalue.width;
	height = rvalue.height;
	offset = rvalue.offset;
	shader = rvalue.shader;
	window = rvalue.window;
	glyph = rvalue.glyph;
	color = rvalue.color;
	samplers = { Sampler2D(rvalue.samplers[0]) };
	vertex = rvalue.vertex;
	index = rvalue.index;
	uv = rvalue.uv;
	vertexArray = rvalue.vertexArray;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

cppogl::CharRect::~CharRect()
{
	glDeleteBuffers(1, &vertex.buffer);
	vertex.buffer = 0;
	glDeleteBuffers(1, &uv.buffer);
	uv.buffer = 0;
	glDeleteBuffers(1, &index.buffer);
	index.buffer = 0;
	glDeleteVertexArrays(1, &vertexArray);
	vertexArray = 0;
}

void cppogl::CharRect::operator=(CharRect && rvalue)
{
	width = rvalue.width;
	height = rvalue.height;
	offset = rvalue.offset;
	shader = rvalue.shader;
	window = rvalue.window;
	glyph = rvalue.glyph;
	color = rvalue.color;
	samplers = { Sampler2D(rvalue.samplers[0]) };
	vertex = rvalue.vertex;
	index = rvalue.index;
	uv = rvalue.uv;
	vertexArray = rvalue.vertexArray;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

void cppogl::CharRect::render()
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
	glDrawElements(GL_TRIANGLES, index.list.size(), GL_UNSIGNED_SHORT, nullptr);
	checkGLErrors(__LINE__);
}

cppogl::Glyph::Glyph()
{
	this->image = nullptr;
}

cppogl::Glyph::Glyph(FT_Glyph& glyph)
{
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	FT_Bitmap& bitmap = bitmap_glyph->bitmap;
	this->glyph = glyph;
	this->border.width = bitmap.width;
	this->border.height = bitmap.rows;
	this->image = std::make_shared<Image>(Image());
	this->image->width = this->border.width;
	this->image->height = this->border.height;
	this->image->bpp = 1;
	this->image->image = new unsigned char[this->image->width * 4 * this->image->height];
	for (int i = 0; i < this->image->width; i++) {
		for (int j = 0; j < this->image->height; j++) {
			if (i < this->border.width && j < this->border.height) {
				int index = this->image->bpp * (i + j * this->image->width);
				this->image->image[index] = bitmap.buffer[j * this->image->width + i];
			}
			else {
				int index = this->image->bpp * (i + j * this->image->width);
				this->image->image[index] = 0;
			}
		}
	}
}

cppogl::Glyph::~Glyph()
{
}

cppogl::Text::Text()
{
	this->_shader = nullptr;
	this->_window = nullptr;
	this->_font = nullptr;
}

cppogl::Text::Text(sWindow window, sShaderProgram shader, sFont font, std::string text, int fontsize, glm::vec3 position)
{
	this->_shader = shader;
	this->_font = font;
	this->_text = text;
	this->_window = window;
	this->_fontSize = fontsize;
	this->_model.location = glGetUniformLocation(this->_shader->id(), "model");
	if (this->_model.location < 0) {
		throw Exception("failed to locate shader uniform: model", EXCEPT_DETAIL_DEFAULT);
	}
	this->_model.matrix = glm::mat4(1.0f);
	this->_model.matrix[0][3] = position.x;
	this->_model.matrix[1][3] = position.y;
	this->_model.matrix[2][3] = position.z;
	this->generate(text, fontsize);
}

cppogl::Text::~Text()
{
}

void cppogl::Text::generate(std::string text, int fontsize)
{
	if (_font == nullptr || _window == nullptr || _shader == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	if (_font->_generated.find(fontsize) == _font->_generated.end()) {
		_font->generate(fontsize);
	}
	this->_charecters.reserve(text.length());
	float horizontalOffset = 0.0f;
	for (int i = 0; i < text.length(); i++) {
		this->_charecters.push_back(CharRect(_window, _shader, _font->_generated[fontsize][text[i]], glm::vec3(horizontalOffset, 0.0, 0.0)));
		horizontalOffset += (float)_font->_generated[fontsize][text[i]]->border.width / _window->width();
	}
	_fontSize = fontsize;
}

void cppogl::Text::update(std::string text)
{
	_charecters.reserve(text.length());
	float horizontalOffset = 0.0f;
	for (int i = 0; i < text.length(); i++) {
		if (i < _text.length()) {
			if (text[i] != _text[i]) {
				_charecters[i] = CharRect(_window, _shader, _font->_generated[_fontSize][text[i]], glm::vec3(horizontalOffset, _charecters.back().offset.y, _charecters.back().offset.z));
			}
		}
		else {
			_charecters.push_back(CharRect(_window, _shader, _font->_generated[_fontSize][text[i]], glm::vec3(horizontalOffset, _charecters.back().offset.y, _charecters.back().offset.z)));
		}
		horizontalOffset += (float) _charecters[i].glyph->border.width / _window->width();
	}
	_text = text;
}

void cppogl::Text::render()
{
	if (_font == nullptr || _window == nullptr || _shader == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	glUniformMatrix4fv(this->_model.location, 1, GL_FALSE, &this->_model.matrix[0][0]);
	for (int i = 0; i < _charecters.size(); i++) {
		_charecters[i].render();
	}
}
