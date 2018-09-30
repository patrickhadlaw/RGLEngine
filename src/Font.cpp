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
		glyphs[c] = std::make_shared<Glyph>(Glyph(ft_glyph, _face->glyph->metrics));
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
	glm::vec3 scaled = glm::vec3(
		window->normalizeX(window->parseUnit(offset.x, Window::Unit::PT)),
		window->normalizeY(window->parseUnit(offset.y, Window::Unit::PT)),
		offset.z
	);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0) + scaled,
		glm::vec3(width, 0.0, 0.0) + scaled,
		glm::vec3(0.0, height, 0.0) + scaled,
		glm::vec3(width, height, 0.0) + scaled
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
	this->samplers = { Sampler2D(shader, glyph->image, GL_TEXTURE0, Sampler2D::Format{ GL_R8, GL_RED }) };
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
	samplers = rvalue.samplers;
	vertex = rvalue.vertex;
	index = rvalue.index;
	uv = rvalue.uv;
	vertexArray = rvalue.vertexArray;
	rvalue.vertexArray = 0;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

cppogl::CharRect::~CharRect()
{

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
	samplers = rvalue.samplers;
	vertex = rvalue.vertex;
	index = rvalue.index;
	uv = rvalue.uv;
	vertexArray = rvalue.vertexArray;
	rvalue.vertexArray = 0;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

void cppogl::CharRect::recalculate()
{
	float width = window->parseUnit((float)glyph->border.width, Window::Unit::PT) / window->width();
	float height = window->parseUnit((float)glyph->border.height, Window::Unit::PT) / window->height();
	glm::vec3 scaled = glm::vec3(
		window->normalizeX(window->parseUnit(offset.x, Window::Unit::PT)),
		window->normalizeY(window->parseUnit(offset.y, Window::Unit::PT)),
		offset.z
	);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0) + scaled,
		glm::vec3(width, 0.0, 0.0) + scaled,
		glm::vec3(0.0, height, 0.0) + scaled,
		glm::vec3(width, height, 0.0) + scaled
	};
	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data());
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
}

cppogl::Glyph::Glyph()
{
	this->image = nullptr;
}

cppogl::Glyph::Glyph(FT_Glyph& glyph, FT_Glyph_Metrics& metrics)
{
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	FT_Bitmap& bitmap = bitmap_glyph->bitmap;
	this->glyph = glyph;
	this->metrics = metrics;
	FT_Glyph_Get_CBox(this->glyph, FT_GLYPH_BBOX_UNSCALED, &this->bbox);
	this->border.width = bitmap.width;
	this->border.height = bitmap.rows;
	this->image = std::make_shared<Image>(Image());
	this->image->width = next_power_n<2>(this->border.width);
	this->image->height = next_power_n<2>(this->border.height);
	this->image->bpp = 1;
	this->image->image = new unsigned char[this->image->width * this->image->bpp * this->image->height];
	for (int i = 0; i < this->image->width; i++) {
		for (int j = 0; j < this->image->height; j++) {
			int index = this->image->bpp * (i + j * this->image->width);
			if (i < this->border.width && j < this->border.height) {
				this->image->image[index] = bitmap.buffer[j * this->border.width + i];
			}
			else {
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

	this->_window->registerListener("resize", this);

	this->_model.matrix = glm::mat4(1.0f);
	this->_model.matrix[0][3] = position.x;
	this->_model.matrix[1][3] = position.y;
	this->_model.matrix[2][3] = position.z;
	this->generate(text, fontsize);
}

cppogl::Text::~Text()
{
}

void cppogl::Text::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		for (int i = 0; i < _charecters.size(); i++) {
			_charecters[i].recalculate();
		}
	}
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
		sGlyph& glyph = _font->_generated[fontsize][text[i]];
		float verticalOffset = ((float)glyph->bbox.yMin / (glyph->bbox.yMax - glyph->bbox.yMin)) * glyph->border.height;
		this->_charecters.push_back(CharRect(
			_window,
			_shader,
			glyph,
			glm::vec3(horizontalOffset, verticalOffset, 0.0)
		));
		horizontalOffset += glyph->glyph->advance.x >> 16;
	}
	_fontSize = fontsize;
}

void cppogl::Text::update(std::string text)
{
	_charecters.reserve(text.length());
	float horizontalOffset = 0.0f;
	for (int i = 0; i < text.length(); i++) {
		sGlyph glyph = _font->_generated[_fontSize][text[i]];
		if (i < _text.length()) {
			if (text[i] != _text[i]) {
				_charecters[i] = CharRect(_window, _shader, glyph, glm::vec3(horizontalOffset, -(glyph->bbox.yMin / (glyph->bbox.yMax - glyph->bbox.yMin)) * glyph->border.height, _charecters.back().offset.z));
			}
		}
		else {
			_charecters.push_back(CharRect(_window, _shader, glyph, glm::vec3(horizontalOffset, -(glyph->bbox.yMin / (glyph->bbox.yMax - glyph->bbox.yMin)) * glyph->border.height, _charecters.back().offset.z)));
		}
		horizontalOffset += _charecters[i].glyph->glyph->advance.x >> 16;
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
